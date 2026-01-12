#pragma once

#include "Entity.h"
#include <vector>

namespace ului {
namespace ecs {

class Scene;

/**
 * Base class for all systems in the ECS architecture.
 * Systems contain logic that operates on entities with specific components.
 * 
 * Systems should:
 * - Query the Scene for entities with required components
 * - Process those entities in Update()
 * - Not store entity-specific state (use components for that)
 */
class System {
public:
    virtual ~System() = default;
    
    /**
     * Initialize the system
     * Called once when the system is added to the scene
     */
    virtual void Initialize(Scene* scene) { m_scene = scene; }
    
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
    Scene* m_scene = nullptr;
};

} // namespace ecs
} // namespace ului
