#include <spire/net/client.hpp>

namespace spire::net {
Client::Client(
    boost::asio::strand<boost::asio::any_io_executor>&& strand,
    boost::asio::ip::tcp::socket&& socket,
    std::function<void(std::unique_ptr<InMessage>)>&& on_message_received)
    : _connection {
        std::move(strand),
        std::move(socket),
        [this](const Connection::CloseCode code) {
            stop();
        },
        [this](std::vector<std::byte>&& data) {
            if (_is_authenticated) {
                // _heart_beater.beat();
            }

            _on_message_received(std::make_unique<InMessage>(shared_from_this(), std::move(data)));
        }},
    _on_message_received {std::move(on_message_received)} {}

void Client::start(boost::asio::any_io_executor& executor) {
    _connection.open(executor);
}

void Client::stop() {
    _connection.close(Connection::CloseCode::Normal);
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
