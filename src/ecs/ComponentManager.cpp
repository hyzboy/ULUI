#include "ecs/ComponentManager.h"

namespace ului {
namespace ecs {

void ComponentManager::RemoveAllComponents(Entity entity) {
    // Get all component types for this entity
    auto it = m_entityComponents.find(entity);
    if (it == m_entityComponents.end()) {
        return;
    }
    
    // Remove from each component type map
    for (const auto& typeIndex : it->second) {
        auto compIt = m_components.find(typeIndex);
        if (compIt != m_components.end()) {
            compIt->second.erase(entity);
        }
    }
    
    // Remove from entity components map
    m_entityComponents.erase(it);
}

} // namespace ecs
} // namespace ului
