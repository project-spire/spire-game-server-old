#pragma once

#include <spire/core/types.hpp>

#include <numbers>

namespace spire {
using meter = f32;
using imeter = i32;

using degree = f32;
using radian = f32;

static degree rad2deg(const radian x) {
    static constexpr float ratio {180.0f / std::numbers::pi};
    return x * ratio;
}

static radian deg2rad(const degree x) {
    static constexpr float ratio {std::numbers::pi / 180.0f};
    return x * ratio;
}


class Speed {
public:
    explicit constexpr Speed(const f32 value)
        : _value {value} {}

    [[nodiscard]] constexpr f32 value() const { return _value; }

    constexpr Speed& operator+=(const Speed& other) {
        _value += other._value;
        return *this;
    }

    constexpr Speed& operator-=(const Speed& other) {
        _value -= other._value;
        return *this;
    }

    constexpr Speed operator-() const {
        return Speed {-_value};
    }

    constexpr Speed operator+(const Speed& other) const {
        return Speed {_value + other._value};
    }

    constexpr Speed operator-(const Speed& other) const {
        return Speed {_value - other._value};
    }

    constexpr Speed operator*(const f32 scalar) const {
        return Speed {_value * scalar};
    }

    constexpr Speed operator/(const f32 scalar) const {
        return Speed {_value / scalar};
    }

    constexpr meter operator*(const duration<f32> sec) const {
        return meter {_value * sec.count()};
    }

    constexpr bool operator==(const Speed& other) const = default;
    constexpr auto operator<=>(const Speed& other) const = default;

private:
    // m/s
    f32 _value {};
};


class Acceleration {
public:
    explicit constexpr Acceleration(const f32 value)
        : _value {value} {}

    [[nodiscard]] constexpr f32 value() const { return _value; }

    constexpr Acceleration& operator+=(const Acceleration& other) {
        _value += other._value;
        return *this;
    }

    constexpr Acceleration& operator-=(const Acceleration& other) {
        _value -= other._value;
        return *this;
    }

    constexpr Acceleration operator-() const {
        return Acceleration {-_value};
    }

    constexpr Acceleration operator+(const Acceleration& other) const {
        return Acceleration {_value + other._value};
    }

    constexpr Acceleration operator-(const Acceleration& other) const {
        return Acceleration {_value - other._value};
    }

    constexpr Acceleration operator*(const f32 scalar) const {
        return Acceleration {_value * scalar};
    }

    constexpr Acceleration operator/(const f32 scalar) const {
        return Acceleration {_value / scalar};
    }

    constexpr Speed operator*(const duration<f32> sec) const {
        return Speed {_value * sec.count()};
    }

    constexpr bool operator==(const Acceleration& other) const = default;
    constexpr auto operator<=>(const Acceleration& other) const = default;

private:
    // m/s**2
    f32 _value {};
};
}