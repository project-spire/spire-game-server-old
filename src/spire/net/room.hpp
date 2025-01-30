#pragma once

#include <spdlog/spdlog.h>
#include <spire/container/concurrent_queue.hpp>
#include <spire/net/client.hpp>
#include <spire/handler/handler_controller.hpp>

#include <ranges>

namespace spire::net {
template <typename ClientType>
class Room;

using TcpRoom = Room<TcpClient>;
using SslRoom = Room<SslClient>;


template <typename ClientType>
class Room : public std::enable_shared_from_this<Room<ClientType>>, boost::noncopyable {
    enum class State : u8 {
        Idle,
        Active,
        Terminating,
    };

public:
    Room(u32 id, boost::asio::any_io_executor& io_executor);
    virtual ~Room();

    void start();
    void stop();
    void terminate();

    void add_client_deferred(std::shared_ptr<ClientType> client);
    void remove_client_deferred(std::shared_ptr<ClientType> client);

    void post_task(std::function<void()>&& task);
    void broadcast_message_deferred(std::shared_ptr<OutMessage> message);

    u32 id() const { return _id; }

private:
    virtual void on_started() {}
    virtual void on_stopped() {}
    virtual void on_terminated() {}

    virtual void on_client_entered(const std::shared_ptr<ClientType>& /*client*/) {}
    virtual void on_client_left(const std::shared_ptr<ClientType>& /*client*/) {}

    void update(time_point<steady_clock> last_update_time);
    virtual void update_internal(time_point<steady_clock> /*now*/, f32 /*dt*/) {}

protected:
    HandlerController<ClientType> _handler_controller {};

private:
    const u32 _id;
    std::atomic<State> _state {State::Idle};

    boost::asio::any_io_executor& _io_executor;

    std::unordered_map<std::shared_ptr<ClientType>, typename ClientType::Signals> _clients {};
    ConcurrentQueue<std::function<void()>> _tasks {};
    MessageQueue<ClientType> _messages {};
};


template <typename ClientType>
Room<ClientType>::Room(const u32 id, boost::asio::any_io_executor& io_executor)
    : _id {id}, _io_executor {io_executor} {}

template <typename ClientType>
Room<ClientType>::~Room() {
    terminate();
}

template <typename ClientType>
void Room<ClientType>::start() {
    if (_state == State::Terminating) return;
    if (_state.exchange(State::Active) == State::Active) return;

    post(_io_executor, [self = this->shared_from_this()] {
        self->update(steady_clock::now());
    });

    on_started();
}

template <typename ClientType>
void Room<ClientType>::stop() {
    if (_state == State::Terminating) return;
    if (_state.exchange(State::Idle) == State::Idle) return;

    on_stopped();
}

template <typename ClientType>
void Room<ClientType>::terminate() {
    if (_state.exchange(State::Terminating) == State::Terminating) return;

    // TODO: Cleanup <- Caution: Data race with update function

    on_terminated();
}

template <typename ClientType>
void Room<ClientType>::add_client_deferred(std::shared_ptr<ClientType> client) {
    if (!client) return;

    if (_clients.empty()) {
        start();
    }

    _tasks.push([this, new_client = std::move(client)] mutable {
        if (new_client->state() == ClientType::State::Terminating) return;

        _clients[new_client] = new_client->bind(
            &_messages,
            [this](std::shared_ptr<ClientType> stopped_client, const typename ClientType::StopCode code) {
                if (code != ClientType::StopCode::Normal) {
                    spdlog::debug("Client(TODO) stopped abnormally with code {}", std::to_underlying(code));
                }

                remove_client_deferred(stopped_client);
            });

        on_client_entered(std::move(new_client));
    });
}

template <typename ClientType>
void Room<ClientType>::remove_client_deferred(std::shared_ptr<ClientType> client) {
    if (!client) return;

    _tasks.push([this, client = std::move(client)] mutable {
        if (!_clients.erase(client)) return;

        on_client_left(std::move(client));
    });
}

template <typename ClientType>
void Room<ClientType>::post_task(std::function<void()>&& task) {
    _tasks.push(std::move(task));
}

template <typename ClientType>
void Room<ClientType>::broadcast_message_deferred(std::shared_ptr<OutMessage> message) {
    _tasks.push([self = this->shared_from_this(), message = std::move(message)] {
        for (const auto& client : self->_clients | std::views::keys)
            client->send(message);
    });
}

template <typename ClientType>
void Room<ClientType>::update(const time_point<steady_clock> last_update_time) {
    if (_state == State::Terminating) return;

    // TODO: IO threads are handling messages and tasks
    // -> Let work threads handle these
    std::queue<std::pair<std::shared_ptr<ClientType>, std::unique_ptr<InMessage>>> messages;
    _messages.swap(messages);
    while (!messages.empty()) {
        auto [client, message] = std::move(messages.front());
        _handler_controller.handle(client, std::move(message));
        messages.pop();
    }

    std::queue<std::function<void()>> tasks;
    _tasks.swap(tasks);
    while (!tasks.empty()) {
        tasks.front()();
        tasks.pop();
    }

    if (_clients.empty() && _state == State::Active) {
        stop();
        return;
    }

    const auto now {steady_clock::now()};
    const f32 dt {duration<f32, std::milli> {now - last_update_time}.count()};

    update_internal(now, dt);

    defer(_io_executor, [self = this->shared_from_this(), now] {
        self->update(now);
    });
}
}
