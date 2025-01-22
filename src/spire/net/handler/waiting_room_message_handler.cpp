#include <spire/net/client.hpp>
#include <spire/net/handler/waiting_room_message_handler.hpp>
#include <spire/msg/base_message.pb.h>

namespace spire::net {
using namespace spire::msg;

void handle_login(Client& client, const Login& login);

std::function<bool(std::unique_ptr<InMessage>)> WaitingRoomMessageHandler::make() {
    return [](std::unique_ptr<InMessage> message) {
        return handle_message(std::move(message));
    };
}

bool WaitingRoomMessageHandler::handle_message(const InMessage& message) {
    BaseMessage base {};
    if (!base.ParseFromArray(message.data(), static_cast<int>(message.size()))) {
        message.client().stop(Client::StopCode::InvalidInMessage);
    }

    switch (base.message_case()) {
        case BaseMessage::kLogin:
            handle_login(message.client(), base.login());
            return true;

        default:
            return false;
    }
}

void handle_login(Client& client, const Login& login) {


    //TODO: authenticate token

    client.authenticate();
}
}
