#pragma once

#include "Entity.h"
#include <vector>

namespace ului {
namespace ecs {

class World;

/**
 * Base class for all systems in the ECS architecture.
 * Systems contain logic that operates on entities with specific components.
 * 
 * Systems should:
 * - Query the World for entities with required components
 * - Process those entities in Update()
 * - Not store entity-specific state (use components for that)
 */
class System {
public:
    virtual ~System() = default;
    
    /**
     * Initialize the system
     * Called once when the system is added to the world
     */
    virtual void Initialize(World* world) { m_world = world; }
    
    /**
     * Update the system
     * Called every frame to process entities
     * @param deltaTime Time elapsed since last update in seconds
     */
    virtual void Update(float deltaTime) = 0;
    
    /**
     * Cleanup when system is removed
     */
    virtual void Shutdown() {}
    
protected:
    World* m_world = nullptr;
};

} // namespace ecs
} // namespace ului
