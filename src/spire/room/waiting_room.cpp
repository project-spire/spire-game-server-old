#include <spdlog/spdlog.h>
#include <spire/handler/auth_handler.hpp>
#include <spire/room/waiting_room.hpp>

namespace spire {
WaitingRoom::WaitingRoom(boost::asio::any_io_executor& io_executor)
    : Room {0, io_executor} {
    _handler_controller.add_handler(AuthHandler::make());
}

void WaitingRoom::on_client_entered(const std::shared_ptr<net::Client>& client) {
    client->start();

    msg::BaseMessage base {};
    base.set_allocated_login_available(new msg::LoginAvailable);

    client->send(std::make_unique<net::OutMessage>(base));
}
}
