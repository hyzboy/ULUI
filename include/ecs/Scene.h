#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"
#include "ComponentManager.h"
#include "TransformDataStorage2D.h"
#include <memory>
#include <vector>
#include <algorithm>

namespace ului {
namespace ecs {

/**
 * The Scene manages all entities, components, and systems in the ECS.
 * It provides the main interface for creating entities, adding components,
 * and updating systems.
 * 
 * Example usage:
 *   Scene scene;
 *   Entity entity = scene.CreateEntity();
 *   scene.AddComponent<Transform2D>(entity, std::make_unique<Transform2D>());
 *   scene.AddSystem(std::make_unique<RenderSystem>());
 *   scene.Update(deltaTime);
 */
class Scene {
public:
    Scene() : m_nextEntityId(0) {}
    ~Scene() = default;
    
    /**
     * Create a new entity
     * @return The newly created entity ID
     */
    Entity CreateEntity() {
        Entity entity = m_nextEntityId++;
        m_entities.push_back(entity);
        return entity;
    }
    
    /**
     * Destroy an entity and all its components
     */
    void DestroyEntity(Entity entity) {
        // Remove all components
        m_componentManager.RemoveAllComponents(entity);
        
        // Remove from entity list
        m_entities.erase(
            std::remove(m_entities.begin(), m_entities.end(), entity),
            m_entities.end()
        );
    }
    
    /**
     * Add a component to an entity
     */
    template<typename T>
    bool AddComponent(Entity entity, std::unique_ptr<T> component) {
        return m_componentManager.AddComponent<T>(entity, std::move(component));
    }
    
    /**
     * Get a component from an entity
     */
    template<typename T>
    T* GetComponent(Entity entity) {
        return m_componentManager.GetComponent<T>(entity);
    }
    
    /**
     * Check if an entity has a component
     */
    template<typename T>
    bool HasComponent(Entity entity) const {
        return m_componentManager.HasComponent<T>(entity);
    }
    
    /**
     * Remove a component from an entity
     */
    template<typename T>
    bool RemoveComponent(Entity entity) {
        return m_componentManager.RemoveComponent<T>(entity);
    }
    
    /**
     * Get all entities with a specific component type
     */
    template<typename T>
    std::vector<Entity> GetEntitiesWithComponent() const {
        return m_componentManager.GetEntitiesWithComponent<T>();
    }
    
    /**
     * Add a system to the scene
     */
    void AddSystem(std::unique_ptr<System> system) {
        system->Initialize(this);
        m_systems.push_back(std::move(system));
    }
    
    /**
     * Update all systems
     * @param deltaTime Time elapsed since last update in seconds
     */
    void Update(float deltaTime) {
        for (auto& system : m_systems) {
            system->Update(deltaTime);
        }
    }
    
    /**
     * Get all entities in the scene
     */
    const std::vector<Entity>& GetAllEntities() const {
        return m_entities;
    }
    
    /**
     * Get the component manager (for advanced usage)
     */
    ComponentManager& GetComponentManager() {
        return m_componentManager;
    }
    
    /**
     * Get the Transform2D data storage (SOA storage)
     */
    TransformDataStorage2D& GetTransformStorage2D() {
        return m_transformStorage2D;
    }
    
private:
    Entity m_nextEntityId;
    std::vector<Entity> m_entities;
    ComponentManager m_componentManager;
    std::vector<std::unique_ptr<System>> m_systems;
    
    // SOA storage for Transform2D data
    TransformDataStorage2D m_transformStorage2D;
};

} // namespace ecs
} // namespace ului
