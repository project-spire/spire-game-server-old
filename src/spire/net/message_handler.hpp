#pragma once

#include <spire/net/message.hpp>

namespace spire::net {
class MessageHandler {
public:
    enum class ResultCode {
        Continue,
        Break,
        Error
    };

    MessageHandler(MessageHandler&& other) noexcept;
    MessageHandler& operator=(MessageHandler&& other) noexcept;

    void add_handler(std::function<ResultCode(const InMessage&)>&& handler);
    void handle_message(std::unique_ptr<InMessage> message) const;

private:
    std::list<std::function<ResultCode(const InMessage&)>> _handlers {};
};


inline MessageHandler::MessageHandler(MessageHandler &&other) noexcept
    : _handlers {std::move(other._handlers)} {}

inline MessageHandler & MessageHandler::operator=(MessageHandler &&other) noexcept {
    if (this != &other) {
        _handlers = std::move(other._handlers);
    }
    return *this;
}

inline void MessageHandler::add_handler(std::function<ResultCode(const InMessage&)>&& handler) {
    _handlers.push_back(std::move(handler));
}

inline void MessageHandler::handle_message(std::unique_ptr<InMessage> message) const {
    for (const auto& handler : _handlers) {
        const auto result {handler(*message)};
        if (result == ResultCode::Break || result == ResultCode::Error) break;
    }
}
}
