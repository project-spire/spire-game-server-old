#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <spire/net/message.hpp>

namespace spire::net {
class Connection final : boost::noncopyable {
public:
    enum class CloseCode : u8 {
        Normal,
        ReceiveError,
        SendError
    };

    Connection(
        boost::asio::ip::tcp::socket&& socket,
        std::function<void(CloseCode)>&& on_closed,
        std::function<void(std::vector<std::byte>&&)>&& on_received);
    ~Connection();

    void open();
    void close(CloseCode code);

    void send(std::unique_ptr<OutMessage> message);
    void send(std::shared_ptr<OutMessage> message);

private:
    boost::asio::awaitable<void> receive();

    boost::asio::strand<boost::asio::any_io_executor> _strand;
    boost::asio::ip::tcp::socket _socket;
    std::atomic<bool> _is_open {false};

    std::function<void(CloseCode)> _on_closed;
    std::function<void(std::vector<std::byte>&&)> _on_received;
};
}
