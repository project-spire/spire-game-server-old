#pragma once

#include <spire/container/concurrent_queue.hpp>
#include <spire/net/connection.hpp>
#include <spire/net/heartbeat.hpp>
#include <spire/net/message.hpp>

namespace spire::net {
template <typename SocketType>
class Client;

using TcpClient = Client<TcpSocket>;
using SslClient = Client<SslSocket>;

enum class ClientStopCode : u8 {
    Normal,
    InvalidInMessage,
    ConnectionError,
    HeartbeatDead,
    AuthenticationError
};


template <typename SocketType>
class Client final : public std::enable_shared_from_this<Client<SocketType>>, boost::noncopyable {
public:
    Client(
        SocketType&& socket,
        std::function<void(std::shared_ptr<Client>)>&& on_stop);
    ~Client();

    static std::shared_ptr<Client> make(
        SocketType&& socket,
        std::function<void(std::shared_ptr<Client>)>&& on_stop);

    void start();
    void stop(ClientStopCode code);

    void send(std::unique_ptr<OutMessage> message);
    void send(std::shared_ptr<OutMessage> message);

    void authenticate(u32 account_id, u32 character_id);

    u64 account_id() const { return _account_id; }
    u64 character_id() const { return _character_id; }
    milliseconds ping() const { return _ping.load(); }
    void set_message_queue(ConcurrentQueue<InMessage>* message_queue) { _message_queue = message_queue; }

    std::string display() const;

private:
    u64 _account_id {};
    u64 _character_id {};
    std::atomic<bool> _is_running {false};
    bool _is_authenticated {false};

    boost::asio::strand<boost::asio::any_io_executor> _strand;

    Connection<SocketType> _connection;
    std::atomic<ConcurrentQueue<InMessage>*> _message_queue {};
    Heartbeat _heartbeat;
    std::atomic<milliseconds> _ping {};

    std::function<void(std::shared_ptr<Client>)> _on_stop;
};


template <typename SocketType>
Client<SocketType>::Client(
    SocketType&& socket,
    std::function<void(std::shared_ptr<Client>)>&& on_stop)
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
              stop(ClientStopCode::HeartbeatDead);
          }},
    _on_stop {std::move(on_stop)} {}

template <typename SocketType>
Client<SocketType>::~Client() {
    stop(ClientStopCode::Normal);
}

template <typename SocketType>
std::shared_ptr<Client<SocketType>> Client<SocketType>::make(
    SocketType&& socket,
    std::function<void(std::shared_ptr<Client>)>&& on_stop) {
    auto client {std::make_shared<Client>(std::move(socket), std::move(on_stop))};

    client->_connection.init(
        [self = client->shared_from_this()](const ConnectionCloseCode code) {
            self->stop(code == ConnectionCloseCode::Normal ? ClientStopCode::Normal : ClientStopCode::ConnectionError);
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
    if (_is_running.exchange(true)) return;

    _connection.open();
    _heartbeat.start();
}

template <typename SocketType>
void Client<SocketType>::stop(const ClientStopCode code) {
    if (!_is_running.exchange(false)) return;

    if (code != ClientStopCode::Normal) {
        spdlog::debug("{} stopped abnormally with code {}", display(), std::to_underlying(code));
    }

    _connection.close(ConnectionCloseCode::Normal);
    _heartbeat.stop();

    _on_stop(this->shared_from_this());
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
void Client<SocketType>::authenticate(const u32 account_id, const u32 character_id) {
    _is_authenticated = true;

    _account_id = account_id;
    _character_id = character_id;
}

template <typename SocketType>
std::string Client<SocketType>::display() const {
    return std::format("Client {{ account_id: {}, character_id: {} }}"sv,
        _account_id, _character_id);
}
}
