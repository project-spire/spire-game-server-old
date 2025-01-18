#include <spire/net/connection.hpp>

namespace spire::net {
Connection::Connection(boost::asio::strand<boost::asio::any_io_executor>&& strand,
    boost::asio::ip::tcp::socket&& socket, std::function<void(CloseCode)>&& on_closed,
    std::function<void(std::vector<std::byte>&&)>&& on_received)
    : _strand {std::move(strand)}, _socket {std::move(socket)},
    _on_closed {std::move(on_closed)}, _on_received {std::move(on_received)} {}

void Connection::open(boost::asio::any_io_executor& executor) {
    if (_is_open.exchange(true)) return;

    co_spawn(executor, [this]()->boost::asio::awaitable<void> {
        while (_is_open) {
            co_await receive();
        }
    }, boost::asio::detached);
}

void Connection::close(const CloseCode code) {
    if (!_is_open.exchange(false)) return;

    boost::system::error_code ec;
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    _socket.close(ec);

    _on_closed(code);
}

void Connection::send(std::shared_ptr<OutMessage> message) {
    if (!message) return;

    dispatch(_strand, [this, message = std::move(message)]->boost::asio::awaitable<void> {
        if (!_is_open) co_return;

        const auto [ec, _] = co_await _socket.async_send(
            message->data(), boost::asio::as_tuple(boost::asio::use_awaitable));
        if (ec) close(CloseCode::SendError);
    });
}

boost::asio::awaitable<void> Connection::receive() {
    std::array<std::byte, sizeof(MessageHeader)> header_buffer {};
    if (const auto [ec, _] = co_await async_read(_socket,
        boost::asio::buffer(header_buffer), boost::asio::as_tuple(boost::asio::use_awaitable)); ec) {
        close(CloseCode::ReceiveError);
        co_return;
    }

    auto [body_size] {MessageHeader::deserialize(header_buffer)};
    if (body_size == 0) {
        close(CloseCode::ReceiveError);
        co_return;
    }

    std::vector<std::byte> body_buffer(body_size);
    if (const auto [ec, _] = co_await async_read(_socket,
        boost::asio::buffer(body_buffer),
        boost::asio::as_tuple(boost::asio::use_awaitable)); ec) {
        close(CloseCode::ReceiveError);
        co_return;
    }

    _on_received(std::move(body_buffer));
}
}
