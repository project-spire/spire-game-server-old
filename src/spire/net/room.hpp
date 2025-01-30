#pragma once

#include <entt/entt.hpp>
#include <spire/container/concurrent_queue.hpp>
#include <spire/net/client.hpp>
#include <spire/handler/handler_controller.hpp>
#include <taskflow/taskflow.hpp>

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
    Room(
        u32 id,
        boost::asio::any_io_executor& io_executor,
        tf::Executor* work_executor = nullptr);
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
    virtual void compose_systems(tf::Taskflow& /*taskflow*/, time_point<steady_clock> /*now*/, f32 /*dt*/) {}

protected:
    HandlerController<ClientType> _handler_controller {};
    entt::registry _registry {};

private:
    const u32 _id;
    std::atomic<State> _state {State::Idle};

    boost::asio::any_io_executor& _io_executor;
    tf::Executor* _work_executor;

    ConcurrentQueue<std::function<void()>> _tasks {};
    ConcurrentQueue<std::unique_ptr<InMessage>> _messages {};
    std::unordered_map<u64, std::shared_ptr<ClientType>> _clients {};
};


template <typename ClientType>
Room<ClientType>::Room(
    const u32 id,
    boost::asio::any_io_executor& io_executor,
    tf::Executor* work_executor)
    : _id {id},
    _io_executor {io_executor},
    _work_executor {work_executor} {}

template <typename ClientType>
Room<ClientType>::~Room() {
    stop();
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
    if (_clients.empty()) {
        start();
    }

    _tasks.push([this, client = std::move(client)] mutable {
        _clients[client->id()] = client;

        on_client_entered(client);
    });
}

template <typename ClientType>
void Room<ClientType>::remove_client_deferred(std::shared_ptr<ClientType> client) {
    _tasks.push([this, client = std::move(client)] {
        _clients.erase(client->id());

        on_client_left(client);
    });
}

template <typename ClientType>
void Room<ClientType>::post_task(std::function<void()>&& task) {
    _tasks.push(std::move(task));
}

template <typename ClientType>
void Room<ClientType>::broadcast_message_deferred(std::shared_ptr<OutMessage> message) {
    _tasks.push([self = this->shared_from_this(), message = std::move(message)] {
        for (const auto& client : self->_clients | std::views::values)
            client->send(message);
    });
}

template <typename ClientType>
void Room<ClientType>::update(const time_point<steady_clock> last_update_time) {
    if (_state == State::Terminating) return;

    // TODO: IO threads are handling messages and tasks
    // -> Let work threads handle these
    std::queue<std::unique_ptr<InMessage>> messages;
    _messages.swap(messages);
    while (!messages.empty()) {
        _handler_controller.handle_message(std::move(messages.front()));
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

    if (!_work_executor) {
        //TODO: Use other executor? How?
        defer(_io_executor, [self = this->shared_from_this(), now] {
            self->update(now);
        });
        return;
    }

    tf::Taskflow system_taskflow {};
    compose_systems(system_taskflow, now, dt);

    // Run and update again
    // TODO: The shorter a room's update time, the more it updates --> Uneven updates
    // --> Use co_spawn and sleep for minimum interval rate? <-- Don't use this_thread::sleep because it will block the
    // thread, so that reduces worker in the thread pool
    _work_executor->run(std::move(system_taskflow),
        [self = this->shared_from_this(), now] {
            defer(self->_io_executor, [self, now] { self->update(now); });
        });
}
}