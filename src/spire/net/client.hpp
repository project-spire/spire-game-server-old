#pragma once

#include <boost/signals2.hpp>
#include <spire/container/concurrent_queue.hpp>
#include <spire/net/connection.hpp>
#include <spire/net/heartbeat.hpp>
#include <spire/net/message.hpp>

namespace spire::net {
template <typename SocketType>
class Client;

using TcpClient = Client<TcpSocket>;
using SslClient = Client<SslSocket>;

template <typename ClientType>
using MessageQueue = ConcurrentQueue<std::pair<std::shared_ptr<ClientType>, std::unique_ptr<InMessage>>>;


template <typename SocketType>
class Client final : public std::enable_shared_from_this<Client<SocketType>>, boost::noncopyable {
public:
    enum class State : u8 {
        Idle,
        Active,
        Terminating,
    };

    enum class StopCode : u8 {
        Normal,
        InvalidInMessage,
        ConnectionError,
        HeartbeatDead,
        AuthenticationError
    };

    struct Signals {
        boost::signals2::scoped_connection on_stopped;
    };

    explicit Client(SocketType&& socket);
    ~Client();

    static std::shared_ptr<Client> make(SocketType&& socket);

    void start();
    void stop(StopCode code);

    void send(std::unique_ptr<OutMessage> message);
    void send(std::shared_ptr<OutMessage> message);

    void authenticate();
    Signals bind(
        MessageQueue<Client>* message_queue,
        std::function<void(std::shared_ptr<Client>, StopCode)>&& on_stopped);

    State state() const { return _state; }
    milliseconds ping() const { return _ping; }

private:
    std::atomic<State> _state {State::Idle};
    bool _is_authenticated {false};

    boost::asio::strand<boost::asio::any_io_executor> _strand;

    Connection<SocketType> _connection;
    std::atomic<MessageQueue<Client>*> _message_queue {};
    Heartbeat _heartbeat;
    std::atomic<milliseconds> _ping {};

    boost::signals2::signal<void(std::shared_ptr<Client>, StopCode)> _stopped {};
};


template <typename SocketType>
Client<SocketType>::Client(SocketType&& socket)
    : _strand {make_strand(socket.get_executor())},
    _connection {std::move(socket)},
    _heartbeat {
          socket.get_executor(),
          [this] {
              msg::BaseMessage base;
              base.set_allocated_heartbeat(new msg::Heartbeat);

              send(std::make_unique<OutMessage>(base));
          },
          [this] {
              stop(StopCode::HeartbeatDead);
          }} {}

template <typename SocketType>
Client<SocketType>::~Client() {
    stop(StopCode::Normal);
}

template <typename SocketType>
std::shared_ptr<Client<SocketType>> Client<SocketType>::make(SocketType&& socket) {
    auto client {std::make_shared<Client>(std::move(socket))};

    client->_connection.init(
        [self = client->shared_from_this()](const typename Connection<SocketType>::CloseCode code) {
            self->stop(code == Connection<SocketType>::CloseCode::Normal ? StopCode::Normal : StopCode::ConnectionError);
        },
        [self = client->shared_from_this()](std::vector<std::byte>&& data) {
            if (self->_is_authenticated) {
                self->_heartbeat.reset();
            }

            const auto message_queue {self->_message_queue.load()};
            if (!message_queue) return;
            message_queue->push(std::make_pair(self->shared_from_this(), std::make_unique<InMessage>(std::move(data))));
        });

    return client;
}

template <typename SocketType>
void Client<SocketType>::start() {
    if (_state == State::Terminating) return;
    if (_state.exchange(State::Active) == State::Active) return;

    _connection.open();
    _heartbeat.start();
}

template <typename SocketType>
void Client<SocketType>::stop(const StopCode code) {
    if (_state.exchange(State::Terminating) == State::Terminating) return;

    _connection.close(Connection<SocketType>::CloseCode::Normal);
    _heartbeat.stop();

    _stopped(this->shared_from_this(), code);
}

template <typename SocketType>
void Client<SocketType>::send(std::unique_ptr<OutMessage> message) {
    _connection.send(std::move(message));
}

template <typename SocketType>
void Client<SocketType>::send(std::shared_ptr<OutMessage> message) {
    _connection.send(std::move(message));
}

template <typename SocketType>
void Client<SocketType>::authenticate() {
    _is_authenticated = true;
}

template <typename SocketType>
typename Client<SocketType>::Signals Client<SocketType>::bind(
    MessageQueue<Client>* message_queue,
    std::function<void(std::shared_ptr<Client>, StopCode)>&& on_stopped) {
    _message_queue = message_queue;

    return Signals {_stopped.connect(std::move(on_stopped))};
}
}
