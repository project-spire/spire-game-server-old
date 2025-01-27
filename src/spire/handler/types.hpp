#pragma once

#include <spire/msg/base_message.pb.h>
#include <spire/net/client.hpp>

namespace spire {
enum class HandlerResult {
    Continue,
    Break,
    Error
};

using HandlerFunction = std::function<HandlerResult(const std::shared_ptr<net::Client>&, const msg::BaseMessage&)>;
}