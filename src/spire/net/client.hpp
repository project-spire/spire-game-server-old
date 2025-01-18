#pragma once

#include <spire/net/connection.hpp>
#include <spire/net/message.hpp>

namespace spire::net {
class Client final : public std::enable_shared_from_this<Client>, boost::noncopyable {
public:
    Client(
        boost::asio::strand<boost::asio::any_io_executor>&& strand,
        boost::asio::ip::tcp::socket&& socket,
        std::function<void(std::unique_ptr<InMessage>)>&& on_message_received);

    void start(boost::asio::any_io_executor& executor);
    void stop();

    void send(std::unique_ptr<OutMessage> message);
    void send(std::shared_ptr<OutMessage> message);

    void authenticate();

private:
    Connection _connection;
    std::function<void(std::unique_ptr<InMessage>)> _on_message_received;

    bool _is_authenticated {false};
};
}