#pragma once

#include "Entity.h"
#include "Component.h"
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <algorithm>

namespace ului {
namespace ecs {

/**
 * Manages storage and retrieval of components for entities.
 * Uses a map-based approach for simplicity (can be optimized later with pools/arrays).
 */
class ComponentManager {
public:
    ComponentManager() = default;
    ~ComponentManager() = default;
    
    /**
     * Add a component to an entity
     * @param entity The entity to add the component to
     * @param component The component to add (ownership is transferred)
     * @return true if successfully added, false if entity already has this component type
     */
    template<typename T>
    bool AddComponent(Entity entity, std::unique_ptr<T> component) {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
        
        auto typeIndex = GetComponentTypeIndex<T>();
        auto& componentMap = m_components[typeIndex];
        
        // Check if entity already has this component type
        if (componentMap.find(entity) != componentMap.end()) {
            return false;
        }
        
        componentMap[entity] = std::move(component);
        
        // Track which components this entity has
        m_entityComponents[entity].push_back(typeIndex);
        
        return true;
    }
    
    /**
     * Get a component from an entity
     * @param entity The entity to get the component from
     * @return Pointer to the component, or nullptr if not found
     */
    template<typename T>
    T* GetComponent(Entity entity) {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
        
        auto typeIndex = GetComponentTypeIndex<T>();
        auto it = m_components.find(typeIndex);
        if (it == m_components.end()) {
            return nullptr;
        }
        
        auto& componentMap = it->second;
        auto compIt = componentMap.find(entity);
        if (compIt == componentMap.end()) {
            return nullptr;
        }
        
        return static_cast<T*>(compIt->second.get());
    }
    
    /**
     * Check if an entity has a component of given type
     */
    template<typename T>
    bool HasComponent(Entity entity) const {
        auto typeIndex = GetComponentTypeIndex<T>();
        auto it = m_components.find(typeIndex);
        if (it == m_components.end()) {
            return false;
        }
        return it->second.find(entity) != it->second.end();
    }
    
    /**
     * Remove a component from an entity
     */
    template<typename T>
    bool RemoveComponent(Entity entity) {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
        
        auto typeIndex = GetComponentTypeIndex<T>();
        auto it = m_components.find(typeIndex);
        if (it == m_components.end()) {
            return false;
        }
        
        auto& componentMap = it->second;
        auto removed = componentMap.erase(entity) > 0;
        
        if (removed) {
            // Remove from entity's component list
            auto& entityComps = m_entityComponents[entity];
            entityComps.erase(
                std::remove(entityComps.begin(), entityComps.end(), typeIndex),
                entityComps.end()
            );
        }
        
        return removed;
    }
    
    /**
     * Remove all components from an entity
     */
    void RemoveAllComponents(Entity entity);
    
    /**
     * Get all entities that have a specific component type
     */
    template<typename T>
    std::vector<Entity> GetEntitiesWithComponent() const {
        std::vector<Entity> entities;
        auto typeIndex = GetComponentTypeIndex<T>();
        
        auto it = m_components.find(typeIndex);
        if (it != m_components.end()) {
            for (const auto& pair : it->second) {
                entities.push_back(pair.first);
            }
        }
        
        return entities;
    }
    
private:
    // Map of component type -> (entity -> component)
    std::unordered_map<std::type_index, std::unordered_map<Entity, std::unique_ptr<Component>>> m_components;
    
    // Map of entity -> list of component types it has
    std::unordered_map<Entity, std::vector<std::type_index>> m_entityComponents;
};

} // namespace ecs
} // namespace ului
