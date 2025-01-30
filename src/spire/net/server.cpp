#include <spdlog/spdlog.h>
#include <spire/core/settings.hpp>
#include <spire/net/server.hpp>
#include <spire/room/admin_room.hpp>
#include <spire/room/waiting_room.hpp>

namespace spire::net {
Server::Server(boost::asio::any_io_executor&& io_executor)
    : _io_executor {std::move(io_executor)},
    _io_strand {make_strand(_io_executor)},
    _game_acceptor {
        make_strand(_io_executor),
        boost::asio::ip::tcp::endpoint {boost::asio::ip::tcp::v4(), Settings::game_listen_port()}},
    _admin_acceptor {
        make_strand(_io_executor),
        boost::asio::ip::tcp::endpoint {boost::asio::ip::tcp::v4(), Settings::admin_listen_port()}},
    _waiting_room {std::make_shared<WaitingRoom>(_io_executor)},
    _admin_room {std::make_shared<AdminRoom>(_io_executor)} {

    _ssl_context.set_options(
        boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2 |
        boost::asio::ssl::context::no_sslv3 |
        boost::asio::ssl::context::no_tlsv1 |
        boost::asio::ssl::context::no_tlsv1_1 |
        boost::asio::ssl::context::no_tlsv1_2);
    _ssl_context.use_certificate_chain_file(Settings::certificate_file());
    _ssl_context.use_private_key_file(Settings::private_key_file(), boost::asio::ssl::context::pem);

    _game_acceptor.set_option(boost::asio::socket_base::reuse_address(true));
    _game_acceptor.listen(Settings::listen_backlog());

    _admin_acceptor.set_option(boost::asio::socket_base::reuse_address(true));
}

Server::~Server() {
    stop();
}

void Server::start() {
    if (_is_running.exchange(true)) return;

    // Spawn game acceptor loop
    co_spawn(_io_executor, [this] -> boost::asio::awaitable<void> {
        spdlog::info("Server listening on game port {}", Settings::game_listen_port());

        while (_is_running) {
            auto [ec, socket] = co_await _game_acceptor.async_accept(boost::asio::as_tuple(boost::asio::use_awaitable));
            if (ec) {
                spdlog::warn("Error accepting game socket");
                continue;
            }

            spdlog::debug("Server accepted game socket from {}", socket.local_endpoint().address().to_string());

            if (socket.set_option(boost::asio::ip::tcp::no_delay(Settings::tcp_no_delay()), ec)) {
                spdlog::warn("Error setting socket option");
                continue;
            }

            _waiting_room->add_client_deferred(TcpClient::make(std::move(socket)));
        }
    }, boost::asio::detached);

    // Spawn admin acceptor loop
    co_spawn(_io_executor, [this] -> boost::asio::awaitable<void> {
        spdlog::info("Server listening on admin port {}", Settings::admin_listen_port());

        while (_is_running) {
            auto [ec, socket] = co_await _game_acceptor.async_accept(boost::asio::as_tuple(boost::asio::use_awaitable));
            if (ec) {
                spdlog::warn("Error accepting admin socket");
                continue;
            }

            spdlog::debug("Server accepted admin socket from {}", socket.local_endpoint().address().to_string());

            SslSocket ssl_socket {std::move(socket), _ssl_context};
            if (co_await ssl_socket.async_handshake(boost::asio::ssl::stream_base::server,
                redirect_error(boost::asio::use_awaitable, ec)); ec) {
                spdlog::warn("Error SSL handshaking");
                continue;
            }

            _admin_room->add_client_deferred(SslClient::make(std::move(ssl_socket)));
        }
    }, boost::asio::detached);
}

void Server::stop() {
    if (!_is_running.exchange(false)) return;

    if (boost::system::error_code ec; _game_acceptor.close(ec)) {
        spdlog::warn("Error closing game acceptor");
    }

    if (boost::system::error_code ec; _admin_acceptor.close(ec)) {
        spdlog::warn("Error closing admin acceptor");
    }

    //TODO: Get future from terminate() and wait
    _waiting_room->terminate();
    _admin_room->terminate();
}
}
