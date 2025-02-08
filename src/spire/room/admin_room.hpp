#pragma once

#include <spire/server/room.hpp>

namespace spire {
class AdminRoom final : public SslRoom {
public:
    explicit AdminRoom(boost::asio::any_io_executor& io_executor);
    ~AdminRoom() override = default;

private:
    void on_client_entered(const std::shared_ptr<net::SslClient>& client) override;
};
}