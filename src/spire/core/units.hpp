#pragma once

#include <numbers>

namespace spire {
using meter = float;
using imeter = int;

using radian = float;
using degree = float;

static degree rad2deg(const radian x) {
    static constexpr float constant {180.0f / std::numbers::pi};
    return x * constant;
}

static radian deg2rad(const degree x) {
    static constexpr float constant {std::numbers::pi / 180.0f};
    return x * constant;
}
}