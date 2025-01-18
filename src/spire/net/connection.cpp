#include <spire/net/connection.hpp>

namespace spire::net {
Connection::Connection(
    boost::asio::strand<boost::asio::any_io_executor>&& strand,
    boost::asio::ip::tcp::socket&& socket)
    : _strand {std::move(strand)}, _socket {std::move(socket)} {

}

bool Connection::open() {}

void Connection::close(CloseCode code) {}

void Connection::send(std::shared_ptr<OutMessage> message) {
    if (!message) return;

    co_spawn(_strand, [this, message = std::move(message)]->boost::asio::awaitable<void> {
        if (!is_open()) co_return;

        const auto [ec, _] = co_await _socket.async_send();
        if (ec) close(CloseCode::SendError);
    }, boost::asio::detached);
}

boost::asio::awaitable<void> Connection::receive() {
    constexpr size_t HEADER_BUFFER_SIZE {4};
    constexpr size_t BODY_BUFFER_MAX_SIZE {65536};

    std::array<std::byte, HEADER_BUFFER_SIZE> header_buffer {};
    if (const auto [ec, _] = co_await async_read(_socket,
        boost::asio::buffer(header_buffer), boost::asio::as_tuple(boost::asio::use_awaitable)); ec) {
        close(CloseCode::ReceiveError);
        co_return;
    }

    const auto body_buffer_size {TODO};
    if (body_buffer_size > BODY_BUFFER_MAX_SIZE || body_buffer_size == 0) {
        close(CloseCode::ReceiveError);
        co_return;
    }

    std::vector<std::byte> body_buffer(body_buffer_size);
    if (const auto [ec, _] = co_await async_read(_socket,
        boost::asio::buffer(body_buffer),
        boost::asio::as_tuple(boost::asio::use_awaitable)); ec) {
        close(CloseCode::ReceiveError);
        co_return;
    }

    TODO:
}
}
