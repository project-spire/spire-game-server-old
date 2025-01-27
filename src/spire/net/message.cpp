#include <spire/net/message.hpp>

#include <bit>
#include <cstring>
#include <limits>
#include <spanstream>
#include <stdexcept>

namespace spire::net {
void MessageHeader::serialize(const MessageHeader& source, std::span<std::byte, SIZE> target) {
    u16 body_size {source.body_size};
    if constexpr (std::endian::native == std::endian::little)
        body_size = std::byteswap(body_size);

    std::memcpy(target.data(), &body_size, sizeof(body_size));
}

MessageHeader MessageHeader::deserialize(const std::span<const std::byte, SIZE> source) {
    u16 body_size;
    std::memcpy(&body_size, source.data(), sizeof(body_size));
    if constexpr (std::endian::native == std::endian::little)
        body_size = std::byteswap(body_size);

    return MessageHeader {.body_size = body_size};
}

InMessage::InMessage(std::shared_ptr<Client> client, std::vector<std::byte>&& data)
    : _client {std::move(client)}, _data {std::move(data)} {}

OutMessage::OutMessage(const MessageHeader header) {
    _data.resize(sizeof(MessageHeader) + header.body_size);

    MessageHeader::serialize(header, std::span<std::byte, sizeof(MessageHeader)> {_data.data(), sizeof(MessageHeader)});
}

OutMessage::OutMessage(const msg::BaseMessage& body) {
    const size_t body_size {body.ByteSizeLong()};

    if (body_size > std::numeric_limits<decltype(MessageHeader::body_size)>::max())
        throw std::length_error("OutMessage body size too large");

    _data.resize(sizeof(MessageHeader) + body_size);

    const MessageHeader header {.body_size = static_cast<u16>(body_size)};
    MessageHeader::serialize(header, std::span<std::byte, sizeof(MessageHeader)> {_data.data(), sizeof(MessageHeader)});

    serialize(body);
}

void OutMessage::serialize(const msg::BaseMessage& body) {
    std::ospanstream os {std::span {
        reinterpret_cast<char*>(_data.data()) + sizeof(MessageHeader), _data.size() - sizeof(MessageHeader)}};

    body.SerializeToOstream(&os);
}
}
