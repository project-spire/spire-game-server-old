#pragma once

#include <spire/net/room.hpp>

namespace spire {
class WaitingRoom final : public net::Room {
public:
    WaitingRoom(
        u32 id,
        boost::asio::any_io_executor& io_executor,
        tf::Executor& system_executor);

private:
    void on_client_entered(const std::shared_ptr<net::Client>& client) override;
};
}