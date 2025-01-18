#pragma once

#include <spire/core/types.hpp>

namespace spire::net {
struct InMessage {
    InMessage(std::vector<std::byte>&& data)
        : _data {std::move(data)} {}

    std::span<std::byte> data() { return std::span {_data.data(), _data.size()}; }

private:
    std::vector<std::byte> _data;
};

struct OutMessage {

};
}