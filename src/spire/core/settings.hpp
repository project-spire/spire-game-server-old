#pragma once

#include <spire/core/types.hpp>

namespace spire {
class Settings final {
public:
    static void init();

    static u16 game_listen_port() { return _game_listen_port; }
    static u16 admin_listen_port() { return _admin_listen_port; }
    static u16 listen_backlog() { return _listen_backlog; }

    static std::string_view auth_jwt_key() { return _auth_jwt_key; }

    static std::string_view db_host() { return _db_host; }
    static uint16_t db_port() { return _db_port; }
    static std::string_view db_name() { return _db_name; }
    static std::string_view db_user() { return _db_user; }
    static std::string_view db_password() { return _db_password; }

    static milliseconds heartbeat_interval() { return _heartbeat_interval; }
    static u8 heartbeat_retries() { return _heartbeat_retries; }

private:
    inline static u16 _game_listen_port;
    inline static u16 _admin_listen_port;
    inline static u16 _listen_backlog;

    inline static std::string _auth_jwt_key;

    inline static std::string _db_host;
    inline static uint16_t _db_port;
    inline static std::string _db_name;
    inline static std::string _db_user;
    inline static std::string _db_password;

    inline static milliseconds _heartbeat_interval;
    inline static u8 _heartbeat_retries;
};
}