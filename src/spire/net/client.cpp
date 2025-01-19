#include <spire/net/client.hpp>

namespace spire::net {
Client::Client(
    boost::asio::ip::tcp::socket&& socket,
    std::function<void(std::unique_ptr<InMessage>)>&& on_message_received,
    std::function<void(std::shared_ptr<Client>)>&& on_stop)
    : _heart_beater {
          socket.get_executor(),
          [this] {
              //TODO: Send HeartBeat message
          },
          [this] {
              stop();
          }},
    _connection {
        std::move(socket),
        [this](const Connection::CloseCode) {
            stop();
        },
        [this](std::vector<std::byte>&& data) {
            if (_is_authenticated) {
                _heart_beater.pulse();
            }

            _on_message_received(std::make_unique<InMessage>(shared_from_this(), std::move(data)));
        }},
    _on_message_received {std::move(on_message_received)},
    _on_stop {std::move(on_stop)} {}

void Client::start() {
    if (_is_running.exchange(true)) return;

    _connection.open();
}

void Client::stop() {
    if (!_is_running.exchange(false)) return;

    _connection.close(Connection::CloseCode::Normal);

    _on_stop(shared_from_this());
}

void Client::send(std::unique_ptr<OutMessage> message) {
    _connection.send(std::move(message));
}

void Client::send(std::shared_ptr<OutMessage> message) {
    _connection.send(std::move(message));
}

void Client::authenticate() {
    _is_authenticated = true;
}
}
