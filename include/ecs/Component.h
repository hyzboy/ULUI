#pragma once

#include <cstdint>
#include <typeindex>
#include <typeinfo>

namespace ului {
namespace ecs {

/**
 * Base interface for all components in the ECS system.
 * Components are pure data containers attached to entities.
 * 
 * This is a marker interface - actual components should be simple structs
 * with data members only (no methods except constructors).
 */
class Component {
public:
    virtual ~Component() = default;
    
    /**
     * Get the type index for this component type
     */
    virtual std::type_index GetTypeIndex() const = 0;
};

/**
 * Helper to get component type ID at compile time
 */
template<typename T>
inline std::type_index GetComponentTypeIndex() {
    return std::type_index(typeid(T));
}

} // namespace ecs
} // namespace ului
