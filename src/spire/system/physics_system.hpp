#pragma once

#include <entt/entt.hpp>
#include <spire/component/physics/transform.hpp>
#include <spire/component/physics/velocity.hpp>
#include <spire/core/units.hpp>


namespace spire::physics {
struct PhysicsConfig {
    Acceleration gravity {9.8};
};


class PhysicsSystem final {
public:
    static void update(entt::registry& registry, f32 dt);
};

inline void PhysicsSystem::update(entt::registry& registry, const f32 dt) {
    registry.view<Transform, Velocity>().each([dt](auto& transform, auto& velocity) {
        transform.position += dt * velocity.v;
    });
}
}
