#pragma once

#include <spire/handler/types.hpp>
#include <spire/net/message.hpp>

namespace spire {
template <typename ClientType>
class HandlerController {
public:
    HandlerController() = default;
    ~HandlerController() = default;
    HandlerController(HandlerController&& other) noexcept;

    void add_handler(HandlerFunction<ClientType>&& handler);
    void handle(
        const std::shared_ptr<ClientType>& client,
        std::unique_ptr<net::InMessage> message) const;

private:
    std::list<HandlerFunction<ClientType>> _handlers {};
};

template <typename ClientType>
HandlerController<ClientType>::HandlerController(HandlerController &&other) noexcept
    : _handlers {std::move(other._handlers)} {}

template <typename ClientType>
void HandlerController<ClientType>::add_handler(HandlerFunction<ClientType>&& handler) {
    _handlers.push_back(std::move(handler));
}

template <typename ClientType>
void HandlerController<ClientType>::handle(
    const std::shared_ptr<ClientType>& client,
    std::unique_ptr<net::InMessage> message) const {
    msg::BaseMessage base {};
    if (!base.ParseFromArray(message->data(), static_cast<int>(message->size()))) {
        client->stop(net::ClientStopCode::InvalidInMessage);
    }

    for (const auto& handler : _handlers) {
        const auto result {handler(client, base)};
        if (result == HandlerResult::Break || result == HandlerResult::Error) break;
    }
}
}
