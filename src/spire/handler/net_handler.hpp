#pragma once

#include <spire/handler/types.hpp>

namespace spire {
class NetHandler final {
public:
    static HandlerFunction<net::TcpClient> make();

private:
    static HandlerResult handle(const std::shared_ptr<net::TcpClient>& client, const msg::BaseMessage& base);
    static HandlerResult handle_ping(const std::shared_ptr<net::TcpClient>& client, const msg::Ping& ping);
};
}