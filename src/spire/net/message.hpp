#pragma once

#include <spire/core/types.hpp>

#include <span>
#include <vector>

namespace spire::net {
struct MessageHeader {
    const u16 body_size;

    static constexpr size_t SIZE = sizeof(decltype(body_size));

    static void serialize(const MessageHeader& source, std::span<std::byte, SIZE> target);
    static MessageHeader deserialize(std::span<const std::byte, SIZE> source);
};

struct InMessage {
    InMessage(Entity entity, std::vector<std::byte>&& data);

    Entity entity() const { return _entity; }
    std::span<const std::byte> data() { return std::span {_data.data(), _data.size()}; }

private:
    const Entity _entity;
    std::vector<std::byte> _data;
};

struct OutMessage {
    explicit OutMessage(MessageHeader header);

    std::span<std::byte> data() { return std::span {_data.data(), _data.size()}; }

private:
    std::vector<std::byte> _data {};
};
}
