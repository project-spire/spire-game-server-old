#include <spire/core/settings.hpp>
#include <spire/net/server.hpp>

namespace spire::net {
Server::Server(boost::asio::any_io_executor&& io_executor)
    : _io_executor {std::move(io_executor)},
    _io_strand {make_strand(_io_executor)},
    _acceptor {
        make_strand(_io_executor),
        boost::asio::ip::tcp::endpoint {boost::asio::ip::tcp::v4(), Settings::listen_port()}},
    _waiting_room {std::make_shared<Room>(0, make_strand(_io_executor), WaitingRoomMessageHandler::make())} {
    _acceptor.set_option(boost::asio::socket_base::reuse_address(true));
    _acceptor.listen(Settings::listen_backlog());
}

Server::~Server() {
    stop();
}

void Server::start() {
    if (_is_running.exchange(true)) return;

    // Spawn acceptor loop
    co_spawn(_io_executor, [this] -> boost::asio::awaitable<void> {
        while (_is_running) {
            auto [ec, socket] = co_await _acceptor.async_accept(boost::asio::as_tuple(boost::asio::use_awaitable));
            if (ec) {
                if (ec == boost::asio::error::operation_aborted) continue;

                //TODO: Warning log
                continue;
            }

            socket.set_option(boost::asio::ip::tcp::no_delay(true), ec);
            if (ec) {
                //TODO: Warning log
                continue;
            }

            add_client_deferred(std::move(socket));
        }
    }, boost::asio::detached);
}

void Server::stop() {
    if (!_is_running.exchange(false)) return;

    {
        boost::system::error_code ec;
        _acceptor.close(ec);
    }

    for (auto& client : _waiting_clients) {
        client->stop(Client::StopCode::Normal);
    }
}

void Server::add_client_deferred(boost::asio::ip::tcp::socket&& socket) {
    auto new_client = std::make_shared<Client>(
        std::move(socket),
        [this](std::shared_ptr<Client> client) mutable {
            remove_client_deferred(std::move(client));
        },
        _waiting_room);

    dispatch(_io_strand, [this, new_client = std::move(new_client)] mutable {
        _waiting_clients.insert(new_client);
        new_client->start();
    });
}

void Server::remove_client_deferred(std::shared_ptr<Client> client) {
    client->stop(Client::StopCode::Normal);

    dispatch(_io_strand, [this, client = std::move(client)] {
        _waiting_clients.erase(client);
    });
}

void Server::add_room_deferred(std::shared_ptr<Room> room) {
    dispatch(_io_strand, [this, room = std::move(room)] mutable {
        _rooms.insert_or_assign(room->id(), std::move(room));
    });
}

void Server::remove_room_deferred(const u32 room_id) {
    dispatch(_io_strand, [this, room_id] {
        _rooms.erase(room_id);
    });
}
}
