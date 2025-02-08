#pragma once

#include <spire/server/room.hpp>

namespace spire {
class WaitingRoom final : public TcpRoom {
public:
    explicit WaitingRoom(boost::asio::any_io_executor& io_executor);
    ~WaitingRoom() override = default;

private:
    void on_client_entered(const std::shared_ptr<net::TcpClient>& client) override;
};
}