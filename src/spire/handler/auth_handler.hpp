#pragma once

#include <spire/handler/types.hpp>

namespace spire {
class AuthHandler final {
public:
    static HandlerFunction make();

private:
    static HandlerResult handle(const std::shared_ptr<net::Client>& client, const msg::BaseMessage& base);
    static HandlerResult handle_login(const std::shared_ptr<net::Client>& client, const msg::Login& login);
};
}