#include <jwt-cpp/jwt.h>
#include <spdlog/spdlog.h>
#include <spire/core/settings.hpp>
#include <spire/handler/auth_handler.hpp>

namespace spire {
HandlerFunction<net::TcpClient> AuthHandler::make() {
    return [](const std::shared_ptr<net::TcpClient>& client, const msg::BaseMessage& base) {
        return handle(client, base);
    };
}

HandlerResult AuthHandler::handle(const std::shared_ptr<net::TcpClient>& client, const msg::BaseMessage& base) {
    switch (base.message_case()) {
    case msg::BaseMessage::kLogin:
        return handle_login(client, base.login());

    default:
        return HandlerResult::Continue;
    }
}

HandlerResult AuthHandler::handle_login(const std::shared_ptr<net::TcpClient>& client, const msg::Login& login) {
    try {
        const auto decoded_token {jwt::decode(login.token())};
        const auto verifier {jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256 {Settings::auth_key().data()})
            .with_claim("account_id", jwt::claim {std::to_string(login.account_id())})
            .with_claim("character_id", jwt::claim {std::to_string(login.character_id())})};

        verifier.verify(decoded_token);
    } catch (const std::exception&) {
        spdlog::warn("Client(TODO): Invalid token");
        client->stop(net::TcpClient::StopCode::AuthenticationError);
        return HandlerResult::Error;
    }

    spdlog::debug("Client(TODO): Authenticated");
    client->authenticate();

    //TODO: Async read player from DB and callback
    {
        // const u64 account_id {}, character_id {};
    }

    return HandlerResult::Break;
}
}
