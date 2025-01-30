#include <spire/room/admin_room.hpp>

namespace spire {
AdminRoom::AdminRoom(boost::asio::any_io_executor& io_executor)
    : Room {0, io_executor} {}

void AdminRoom::on_client_entered(const std::shared_ptr<net::SslClient>& client) {
    client->start();
}
}
