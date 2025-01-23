#pragma once

#include <spire/core/types.hpp>

#include <numbers>

namespace spire {
using meter = f32;
using imeter = i32;

class Degree;

class Radian {
public:
    explicit constexpr Radian(const f32 value)
        : _value {value} {}

    explicit constexpr Radian(const Degree degree)
        : _value {degree.to_radian().value()} {}

    [[nodiscard]] constexpr f32 value() const { return _value; }

    [[nodiscard]] Degree to_degree() const {
        static constexpr f32 ratio {180.0f / std::numbers::pi};
        return Degree {_value * ratio};
    }

    constexpr Radian& operator+=(const Radian& other) {
        _value += other._value;
        return *this;
    }

    constexpr Radian& operator-=(const Radian& other) {
        _value -= other._value;
        return *this;
    }

    constexpr Radian& operator*=(const Radian& other) {
        _value *= other._value;
        return *this;
    }

    constexpr Radian& operator/=(const Radian& other) {
        _value /= other._value;
        return *this;
    }

    constexpr Radian operator-() const {
        return Radian {-_value};
    }

    constexpr Radian operator+(const Radian& other) const {
        return Radian {_value + other._value};
    }

    constexpr Radian operator-(const Radian& other) const {
        return Radian {_value - other._value};
    }

    constexpr Radian operator*(const Radian& other) const {
        return Radian {_value * other._value};
    }

    constexpr Radian operator/(const Radian& other) const {
        return Radian {_value / other._value};
    }

    constexpr bool operator==(const Radian& other) const = default;
    constexpr auto operator<=>(const Radian& other) const = default;

private:
    f32 _value {};
};


class Degree {
public:
    explicit constexpr Degree(const f32 value)
        : _value {value} {}

    explicit constexpr Degree(const Radian radian)
        : _value {radian.to_degree().value()} {}

    [[nodiscard]] constexpr f32 value() const { return _value; }

    [[nodiscard]] Radian to_radian() const {
        static constexpr f32 ratio {std::numbers::pi / 180.0f};
        return Radian {_value * ratio};
    }

    constexpr Degree& operator+=(const Degree& other) {
        _value += other._value;
        return *this;
    }

    constexpr Degree& operator-=(const Degree& other) {
        _value -= other._value;
        return *this;
    }

    constexpr Degree& operator*=(const Degree& other) {
        _value *= other._value;
        return *this;
    }

    constexpr Degree& operator/=(const Degree& other) {
        _value /= other._value;
        return *this;
    }

    constexpr Degree operator-() const {
        return Degree {-_value};
    }

    constexpr Degree operator+(const Degree& other) const {
        return Degree {_value + other._value};
    }

    constexpr Degree operator-(const Degree& other) const {
        return Degree {_value - other._value};
    }

    constexpr Degree operator*(const Degree& other) const {
        return Degree {_value * other._value};
    }

    constexpr Degree operator/(const Degree& other) const {
        return Degree {_value / other._value};
    }

    constexpr bool operator==(const Radian& other) const = default;
    constexpr auto operator<=>(const Radian& other) const = default;

private:
    f32 _value {};
};


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