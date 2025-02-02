#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/core/noncopyable.hpp>
#include <spire/net/message.hpp>

namespace spire::net {
using TcpSocket = boost::asio::ip::tcp::socket;
using SslSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;


template <typename SocketType>
class Connection final : boost::noncopyable {
public:
    enum class CloseCode : u8 {
        Normal,
        ReceiveError,
        SendError
    };

    explicit Connection(SocketType&& socket);
    ~Connection();

    void init(
        std::function<void(CloseCode)>&& on_closed,
        std::function<void(std::vector<std::byte>&&)>&& on_received);
    void open();
    void close(CloseCode code);

    void send(std::unique_ptr<OutMessage> message);
    void send(std::shared_ptr<OutMessage> message);

private:
    boost::asio::awaitable<void> receive();

    boost::asio::strand<boost::asio::any_io_executor> _strand;
    SocketType _socket;
    boost::asio::cancellation_signal _cancelled {};

    std::atomic<bool> _is_open {false};

    std::function<void(CloseCode)> _on_closed;
    std::function<void(std::vector<std::byte>&&)> _on_received;
};


template <typename SocketType>
Connection<SocketType>::Connection(SocketType&& socket)
    : _strand {make_strand(socket.get_executor())}, _socket {std::move(socket)} {}

template <typename SocketType>
Connection<SocketType>::~Connection() {
    close(CloseCode::Normal);
}

template <typename SocketType>
void Connection<SocketType>::init(
    std::function<void(CloseCode)>&& on_closed,
    std::function<void(std::vector<std::byte>&&)>&& on_received) {
    _on_closed = std::move(on_closed);
    _on_received = std::move(on_received);
}

template <typename SocketType>
void Connection<SocketType>::open() {
    if (_is_open.exchange(true)) return;

    // Use cancellation slot because `this` can be dangling after destruction of `Connection`
    co_spawn(_socket.get_executor(), [this] -> boost::asio::awaitable<void> {
        while (_is_open) {
            co_await receive();
        }
    }, bind_cancellation_slot(_cancelled.slot(), boost::asio::detached));
}

template <typename SocketType>
void Connection<SocketType>::close(const CloseCode code) {
    if (!_is_open.exchange(false)) return;

    _cancelled.emit(boost::asio::cancellation_type::all);

    boost::system::error_code ec;

    _socket.lowest_layer().cancel(ec);
    _socket.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    _socket.lowest_layer().close(ec);

    _on_closed(code);
}

template <typename SocketType>
void Connection<SocketType>::send(std::unique_ptr<OutMessage> message) {
    if (!message || message->empty()) return;

    co_spawn(_strand, [this, message = std::move(message)] -> boost::asio::awaitable<void> {
        if (!_is_open) co_return;

        const auto [ec, _] = co_await async_write(
            _socket, message->span(), boost::asio::as_tuple(boost::asio::use_awaitable));
        if (ec) close(CloseCode::SendError);
    }, boost::asio::detached);
}

template <typename SocketType>
void Connection<SocketType>::send(std::shared_ptr<OutMessage> message) {
    if (!message || message->empty()) return;

    co_spawn(_strand, [this, message = std::move(message)] -> boost::asio::awaitable<void> {
        if (!_is_open) co_return;

        const auto [ec, _] = co_await async_write(
            _socket, message->span(), boost::asio::as_tuple(boost::asio::use_awaitable));
        if (ec) close(CloseCode::SendError);
    }, boost::asio::detached);
}

template <typename SocketType>
boost::asio::awaitable<void> Connection<SocketType>::receive() {
    std::array<std::byte, sizeof(MessageHeader)> header_buffer {};
    if (const auto [ec, _] = co_await async_read(
        _socket,
        boost::asio::buffer(header_buffer),
        boost::asio::as_tuple(boost::asio::use_awaitable)); ec) {
        close(CloseCode::ReceiveError);
        co_return;
    }

    auto [body_size] {MessageHeader::deserialize(header_buffer)};
    if (body_size == 0) {
        close(CloseCode::ReceiveError);
        co_return;
    }

    std::vector<std::byte> body_buffer(body_size);
    if (const auto [ec, _] = co_await async_read(
        _socket,
        boost::asio::buffer(body_buffer),
        boost::asio::as_tuple(boost::asio::use_awaitable)); ec) {
        close(CloseCode::ReceiveError);
        co_return;
    }

    _on_received(std::move(body_buffer));
}
}
