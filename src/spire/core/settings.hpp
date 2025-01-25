#pragma once

#include <spire/core/types.hpp>

namespace spire {
class Settings final {
public:
    static void init();

    static u16 listen_port() { return _listen_port; }
    static u16 listen_backlog() { return _listen_backlog; }

    static std::string_view auth_jwt_key() { return _auth_jwt_key; }

private:
    inline static u16 _listen_port;
    inline static u16 _listen_backlog;

    inline static std::string _auth_jwt_key;
};
}