#pragma once

#include <spire/core/heartbeater.hpp>
#include <spire/net/connection.hpp>
#include <spire/net/message.hpp>

namespace spire::net {
class Client final : std::enable_shared_from_this<Client>, boost::noncopyable {
public:
    Client(
        boost::asio::ip::tcp::socket&& socket,
        std::function<void(std::unique_ptr<InMessage>)>&& on_message_received,
        std::function<void(std::shared_ptr<Client>)>&& on_stop);

    void start();
    void stop();

    void send(std::unique_ptr<OutMessage> message);
    void send(std::shared_ptr<OutMessage> message);

    void authenticate();

private:
    Heartbeater _heartbeater;
    Connection _connection;
    std::function<void(std::unique_ptr<InMessage>)> _on_message_received;
    std::function<void(std::shared_ptr<Client>)> _on_stop;

    std::atomic<bool> _is_running {false};
    bool _is_authenticated {false};
};
}