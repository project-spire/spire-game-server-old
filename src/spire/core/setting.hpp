#pragma once

#include <spire/core/types.hpp>

namespace spire {
class Setting final {
public:
    static void init();

    static u16 listen_port() { return _listen_port; }
    static u16 listen_backlog() { return _listen_backlog; }

private:
    inline static u16 _listen_port;
    inline static u16 _listen_backlog;
};
}