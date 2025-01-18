#pragma once

#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <spire/net/message.hpp>

namespace spire::net {
class Connection final : boost::noncopyable {
public:
    enum class CloseCode {
        Normal,
        ReceiveError,
        SendError
    };

    Connection(
        boost::asio::strand<boost::asio::any_io_executor>&& strand,
        boost::asio::ip::tcp::socket&& socket);

    bool open();
    void close(CloseCode code);
    void send(std::shared_ptr<OutMessage> message);

    bool is_open() const;
    bool is_connected() const;

private:
    boost::asio::awaitable<void> receive();

    boost::asio::strand<boost::asio::any_io_executor> _strand;
    boost::asio::ip::tcp::socket _socket;
};
}
