#pragma once

#include <glm/vec3.hpp>
#include <spire/core/units.hpp>

namespace spire::physics {
struct Transform {
    glm::vec3 position;
    radian rotation;
};
}