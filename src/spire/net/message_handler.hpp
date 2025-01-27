#pragma once

#include <spire/handler/types.hpp>
#include <spire/net/message.hpp>

namespace spire::net {
class MessageHandler {
public:
    MessageHandler(MessageHandler&& other) noexcept;
    MessageHandler& operator=(MessageHandler&& other) noexcept;

    void add_handler(HandlerFunction&& handler);
    void handle_message(std::unique_ptr<InMessage> message) const;

private:
    std::list<HandlerFunction> _handlers {};
};


inline MessageHandler::MessageHandler(MessageHandler &&other) noexcept
    : _handlers {std::move(other._handlers)} {}

inline MessageHandler & MessageHandler::operator=(MessageHandler &&other) noexcept {
    if (this != &other) {
        _handlers = std::move(other._handlers);
    }
    return *this;
}

inline void MessageHandler::add_handler(HandlerFunction&& handler) {
    _handlers.push_back(std::move(handler));
}

inline void MessageHandler::handle_message(std::unique_ptr<InMessage> message) const {
    msg::BaseMessage base {};
    if (!base.ParseFromArray(message->data(), static_cast<int>(message->size()))) {
        message->client()->stop(Client::StopCode::InvalidInMessage);
    }

    for (const auto& handler : _handlers) {
        const auto result {handler(message->client(), base)};
        if (result == HandlerResult::Break || result == HandlerResult::Error) break;
    }
}
}
