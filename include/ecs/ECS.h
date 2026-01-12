#pragma once

/**
 * ULUI Entity Component System (ECS) for 2D
 * 
 * This ECS implementation is designed specifically for 2D applications.
 * Components use a "2D" suffix (e.g., Transform2D, Sprite2D) to distinguish
 * them from potential future 3D components and to make the API clear.
 * 
 * Quick Start Example:
 * 
 *     #include "ecs/ECS.h"
 *     using namespace ului::ecs;
 * 
 *     // Create a scene
 *     Scene scene;
 * 
 *     // Create an entity
 *     Entity player = scene.CreateEntity();
 * 
 *     // Add components
 *     auto transform = std::make_unique<Transform2D>(&scene.GetTransformStorage2D(), 100.0f, 200.0f);
 *     scene.AddComponent(player, std::move(transform));
 * 
 *     auto sprite = std::make_unique<Sprite2D>("player.png");
 *     sprite->SetSize(64.0f, 64.0f);
 *     scene.AddComponent(player, std::move(sprite));
 * 
 *     auto renderable = std::make_unique<Renderable2D>(true, 1);
 *     scene.AddComponent(player, std::move(renderable));
 * 
 *     // Access components
 *     Transform2D* t = scene.GetComponent<Transform2D>(player);
 *     if (t) {
 *         t->Translate(10.0f, 0.0f);
 *     }
 * 
 *     // Create and add systems
 *     scene.AddSystem(std::make_unique<RenderSystem2D>());
 * 
 *     // Update (in game loop)
 *     scene.Update(deltaTime);
 * 
 * Architecture:
 * - Entity: Lightweight ID representing a game object
 * - Component: Pure data (e.g., Transform2D, Sprite2D, Renderable2D)
 * - System: Logic that processes entities with specific components
 * - Scene: Container managing all entities, components, and systems
 */

// Core ECS
#include "Entity.h"
#include "Component.h"
#include "System.h"
#include "Scene.h"
#include "ComponentManager.h"

// 2D Components
#include "components/Transform2D.h"
#include "components/Sprite2D.h"
#include "components/Renderable2D.h"

namespace ului {
namespace ecs {

/**
 * Helper function to create an entity with Transform2D component
 */
inline Entity CreateEntity2D(Scene& scene, float x = 0.0f, float y = 0.0f) {
    Entity entity = scene.CreateEntity();
    scene.AddComponent(entity, std::make_unique<Transform2D>(&scene.GetTransformStorage2D(), x, y));
    return entity;
}

/**
 * Helper function to create a sprite entity with all common 2D components
 */
inline Entity CreateSpriteEntity(Scene& scene, const std::string& texturePath, 
                                  float x = 0.0f, float y = 0.0f,
                                  float width = 0.0f, float height = 0.0f) {
    Entity entity = scene.CreateEntity();
    
    // Add Transform2D
    scene.AddComponent(entity, std::make_unique<Transform2D>(&scene.GetTransformStorage2D(), x, y));
    
    // Add Sprite2D
    auto sprite = std::make_unique<Sprite2D>(texturePath);
    if (width > 0.0f && height > 0.0f) {
        sprite->SetSize(width, height);
    }
    scene.AddComponent(entity, std::move(sprite));
    
    // Add Renderable2D
    scene.AddComponent(entity, std::make_unique<Renderable2D>(true, 0));
    
    return entity;
}

} // namespace ecs
} // namespace ului
