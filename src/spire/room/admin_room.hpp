#pragma once

#include <spire/net/room.hpp>

namespace spire {
class AdminRoom final : public net::Room {
public:
    explicit AdminRoom(boost::asio::any_io_executor& io_executor);

private:
    void on_client_entered(const std::shared_ptr<net::Client>& client) override;
};
}