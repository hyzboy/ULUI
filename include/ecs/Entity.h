#pragma once

#include <cstdint>
#include <limits>

namespace ului {
namespace ecs {

/**
 * Entity represents a unique identifier in the ECS system.
 * Entities are lightweight IDs that components can be attached to.
 */
using Entity = uint32_t;

/**
 * Special value representing an invalid/null entity
 */
constexpr Entity NULL_ENTITY = std::numeric_limits<Entity>::max();

/**
 * Check if an entity is valid
 */
inline bool IsValidEntity(Entity entity) {
    return entity != NULL_ENTITY;
}

} // namespace ecs
} // namespace ului
