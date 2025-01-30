#pragma once

#include <spire/net/client.hpp>

namespace spire {
enum class HandlerResult {
    Continue,
    Break,
    Error
};

template <typename ClientType>
using HandlerFunction = std::function<HandlerResult(const std::shared_ptr<ClientType>&, const msg::BaseMessage&)>;
}