#pragma once

#include <spire/net/message.hpp>

namespace spire::net {
class WaitingRoomMessageHandler final {
public:
    static std::function<bool(const InMessage&)> make();

    static bool handle_message(const InMessage& message);
};
}