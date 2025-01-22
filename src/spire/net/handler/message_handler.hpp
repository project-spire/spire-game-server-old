#pragma once

#include <spire/net/message.hpp>

namespace spire::net {
class MessageHandler {
public:
    //TODO: move ctor, move assign
    MessageHandler& add_handler(std::function<bool(const InMessage&)>&& handler);

    void handle_message(std::unique_ptr<InMessage> message) const;

private:
    std::list<std::function<bool(const InMessage&)>> _handlers {};
};


inline MessageHandler& MessageHandler::add_handler(std::function<bool(const InMessage&)>&& handler) {
    _handlers.push_back(std::move(handler));

    return *this;
}

inline void MessageHandler::handle_message(std::unique_ptr<InMessage> message) const {
    for (const auto& handler : _handlers)
        if (handler(*message)) return;
}
}
