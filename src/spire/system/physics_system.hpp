#pragma once

#include <entt/entt.hpp>
#include <spire/component/physics/transform.hpp>
#include <spire/component/physics/velocity.hpp>
#include <spire/core/types.hpp>

namespace spire::physics {
class PhysicsSystem final {
public:
    static void update(entt::registry& registry, f32 dt);
};

inline void PhysicsSystem::update(entt::registry& registry, const f32 dt) {
    registry.view<Transform, Velocity>().each([dt](auto& transform, auto& velocity) {
        transform.postion += velocity * dt;
    });
}
}
