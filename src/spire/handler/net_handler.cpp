#include <spire/handler/net_handler.hpp>

namespace spire {
HandlerFunction<net::TcpClient> NetHandler::make() {
    return [](const std::shared_ptr<net::TcpClient>& client, const msg::BaseMessage& base) {
        return handle(client, base);
    };
}

HandlerResult NetHandler::handle(const std::shared_ptr<net::TcpClient>& client, const msg::BaseMessage& base) {
    switch (base.message_case()) {
    case msg::BaseMessage::kPing:
        return handle_ping(client, base.ping());

    default:
        return HandlerResult::Continue;
    }
}

HandlerResult NetHandler::handle_ping(const std::shared_ptr<net::TcpClient>& client, const msg::Ping&) {
    msg::BaseMessage base {};
    base.set_allocated_ping(new msg::Ping);
    client->send(std::make_unique<net::OutMessage>(base));

    return HandlerResult::Break;
}
}
