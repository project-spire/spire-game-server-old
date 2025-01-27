#include <spire/room/waiting_room.hpp>

namespace spire {
WaitingRoom::WaitingRoom(u32 id, boost::asio::any_io_executor& io_executor, tf::Executor& system_executor)
    : Room {id, io_executor, system_executor, } {}

void WaitingRoom::on_client_entered(const std::shared_ptr<net::Client>& client) {
    client->start();

    msg::LoginAvailable login_available;
    msg::BaseMessage base;
    base.set_allocated_login_available(&login_available);

    client->send(std::make_unique<net::OutMessage>(base));
}
}
