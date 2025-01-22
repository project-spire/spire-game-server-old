#pragma once

#include <spire/core/types.hpp>
#include <spire/msg/base_message.pb.h>

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
    InMessage(const InMessage&) = delete;
    InMessage& operator=(const InMessage&) = delete;

    Client& client() const { return *_client; }
    std::span<const std::byte> span() const { return std::span {_data.data(), _data.size()}; }
    const std::byte* data() const { return _data.data(); }
    std::byte* data() { return _data.data(); }
    size_t size() const { return _data.size(); }

private:
    std::shared_ptr<Client> _client;
    std::vector<std::byte> _data;
};

struct OutMessage {
    explicit OutMessage(MessageHeader header);
    explicit OutMessage(const msg::BaseMessage* body);
    ~OutMessage() = default;
    OutMessage(const OutMessage&) = delete;
    OutMessage& operator=(const OutMessage&) = delete;

    void serialize(const msg::BaseMessage* body);

    std::span<const std::byte> span() const { return std::span {_data.data(), _data.size()}; }
    bool empty() const { return _data.empty(); }

private:
    std::vector<std::byte> _data {};
};
}
