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

    _ssl_context.use_certificate_chain_file(TODO);
    _ssl_context.use_private_key_file(TODO, boost::asio::ssl::context::pem);

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
                if (ec == boost::asio::error::operation_aborted) continue;

                spdlog::warn("Error accepting socket");
                continue;
            }

            socket.set_option(boost::asio::ip::tcp::no_delay(true), ec);
            if (ec) {
                spdlog::warn("Error setting socket option");
                continue;
            }

            spdlog::debug("Server accepted game client from {}", socket.local_endpoint().address().to_string());
            add_game_client(std::move(socket));
        }
    }, boost::asio::detached);

    // Spawn admin acceptor loop
    co_spawn(_io_executor, [this] -> boost::asio::awaitable<void> {
        spdlog::info("Server listening on admin port {}", Settings::admin_listen_port());

        while (_is_running) {
            auto [ec, socket] = co_await _game_acceptor.async_accept(boost::asio::as_tuple(boost::asio::use_awaitable));
            if (ec) {
                if (ec == boost::asio::error::operation_aborted) continue;

                spdlog::warn("Error accepting socket");
                continue;
            }

            SslSocket ssl_socket {std::move(socket), _ssl_context};
            auto [ec2] = co_await ssl_socket.async_handshake(boost::asio::as_tuple(boost::asio::use_awaitable));
            if (ec2) {
                spdlog::warn("Error SSL handshaking");
                continue;
            }

            spdlog::debug("Server accepted admin client from {}",
                ssl_socket.next_layer().local_endpoint().address().to_string());
            add_admin_client(std::move(ssl_socket));
        }
    }, boost::asio::detached);
}

void Server::stop() {
    if (!_is_running.exchange(false)) return;

    {
        boost::system::error_code ec;
        _game_acceptor.close(ec);
        _admin_acceptor.close(ec);
    }

    std::promise<void> cleanup_promise {};
    const auto cleanup_future {cleanup_promise.get_future()};

    post(_io_strand, [this, cleanup_promise = std::move(cleanup_promise)] mutable {
        spdlog::info("Server cleanup...");

        for (const auto& client : _clients | std::views::values)
            client->stop(Client::StopCode::Normal);

        _waiting_room->stop();
        _admin_room->stop();
        for (const auto& room : _rooms | std::views::values)
            room->stop();

        cleanup_promise.set_value();
    });
    cleanup_future.wait();

    spdlog::info("Server cleanup done.");
}

void Server::add_game_client(TcpSocket&& socket) {
    //TODO: Use signal for client stop and room manages it
    auto client {TcpClient::make(std::move(socket))};

    dispatch(_io_strand, [this, client = std::move(client)] mutable {
        _clients[client->id()] = client;
        _waiting_room->add_client_deferred(std::move(client));
    });
}

void Server::add_admin_client(SslSocket&& socket) {
    auto client {SslClient::make(std::move(socket))};

    _admin_room->add_client_deferred(std::move(client));
}

void Server::remove_game_client_deferred(std::shared_ptr<GameClient> client) {
    client->stop(ClientStopCode::Normal);

    dispatch(_io_strand, [this, client = std::move(client)] {
        if (const auto current_room {client->current_room().load().lock()}) {
            current_room->remove_client_deferred(client);
        }
        _clients.erase(client->id());
    });
}

void Server::transfer_client_deferred(std::shared_ptr<Client> client, const u32 target_room_id) {
    dispatch(_io_strand, [this, client = std::move(client), target_room_id] {
        if (_rooms.contains(target_room_id)) {
            spdlog::warn("No room with target id {}.", target_room_id);
            return;
        }

        if (const auto current_room {client->current_room().load().lock()}) {
            current_room->remove_client_deferred(client);
        }
        _rooms.at(target_room_id)->add_client_deferred(client);
    });
}
}
