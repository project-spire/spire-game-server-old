#pragma once

#include <spire/server/room.hpp>

namespace spire {
template <typename RoomType>
class District;

using TcpDistrict = District<TcpRoom>;
using SslDistrict = District<SslRoom>;


template <typename RoomType>
class District {
public:
    void broadcast_message(std::shared_ptr<net::OutMessage> message);

private:
    std::unordered_map<u32, std::shared_ptr<RoomType>> _rooms {};
};


template <typename RoomType>
void District<RoomType>::broadcast_message(std::shared_ptr<net::OutMessage> message) {
    for (const auto& room : _rooms | std::views::values)
        room->broadcast_message_deferred(message);
}
}