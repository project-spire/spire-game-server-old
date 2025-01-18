#pragma once

#include <spire/core/types.hpp>

#include <span>
#include <vector>

namespace spire::net {
class Client;

struct MessageHeader {
    const u16 body_size;

    static constexpr size_t SIZE = sizeof(decltype(body_size));

    static void serialize(const MessageHeader& source, std::span<std::byte, SIZE> target);
    static MessageHeader deserialize(std::span<const std::byte, SIZE> source);
};

struct InMessage {
    InMessage(std::shared_ptr<Client> client, std::vector<std::byte>&& data);
    ~InMessage() = default;
    InMessage(InMessage&& other) noexcept;
    InMessage(const InMessage&) = delete;
    InMessage& operator=(const InMessage&) = delete;

    Client* client() const { return _client.get(); }
    std::span<const std::byte> data() const { return std::span {_data.data(), _data.size()}; }

private:
    std::shared_ptr<Client> _client;
    std::vector<std::byte> _data;
};

struct OutMessage {
    explicit OutMessage(MessageHeader header);
    ~OutMessage() = default;
    OutMessage(OutMessage&& other) noexcept;
    OutMessage(const OutMessage&) = delete;
    OutMessage& operator=(const OutMessage&) = delete;

    std::span<const std::byte> data() const { return std::span {_data.data(), _data.size()}; }
    std::span<std::byte> data() { return std::span {_data.data(), _data.size()}; }

private:
    std::vector<std::byte> _data {};
};
}
