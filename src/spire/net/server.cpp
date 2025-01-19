#include <spire/core/setting.hpp>
#include <spire/net/server.hpp>

namespace spire::net {
Server::Server(boost::asio::any_io_executor&& executor)
    : _executor {std::move(executor)},
    _strand {make_strand(_executor)},
    _acceptor {
        make_strand(_executor),
        boost::asio::ip::tcp::endpoint {boost::asio::ip::tcp::v4(), Setting::listen_port()}} {
    _acceptor.set_option(boost::asio::socket_base::reuse_address(true));
    _acceptor.listen(Setting::listen_backlog());
}

Server::~Server() {
    stop();
}

void Server::start() {
    if (_is_running.exchange(true)) return;

    // Spawn acceptor loop
    co_spawn(_executor, [this] -> boost::asio::awaitable<void> {
        while (_is_running) {
            auto [ec, socket] = co_await _acceptor.async_accept(boost::asio::as_tuple(boost::asio::use_awaitable));
            if (ec) {
                if (ec == boost::asio::error::operation_aborted) break;

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
}

void Server::add_client_deferred(boost::asio::ip::tcp::socket&& socket) {
    auto new_client = std::make_shared<Client>(
        std::move(socket),
        [this](std::unique_ptr<InMessage> message) {
            //TODO: Handle message
        },
        [this](std::shared_ptr<Client> client) mutable {
            remove_client_deferred(std::move(client));
        });

    post(_strand, [this, new_client = std::move(new_client)] mutable {
        _clients.insert(new_client);
        new_client->start();
    });
}

void Server::remove_client_deferred(std::shared_ptr<Client> client) {
    client->stop();

    post(_strand, [this, client = std::move(client)] {
        _clients.erase(client);
    });
}
}
