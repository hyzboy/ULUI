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
 *     // Create a world
 *     World world;
 * 
 *     // Create an entity
 *     Entity player = world.CreateEntity();
 * 
 *     // Add components
 *     auto transform = std::make_unique<Transform2D>(100.0f, 200.0f);
 *     world.AddComponent(player, std::move(transform));
 * 
 *     auto sprite = std::make_unique<Sprite2D>("player.png");
 *     sprite->SetSize(64.0f, 64.0f);
 *     world.AddComponent(player, std::move(sprite));
 * 
 *     auto renderable = std::make_unique<Renderable2D>(true, 1);
 *     world.AddComponent(player, std::move(renderable));
 * 
 *     // Access components
 *     Transform2D* t = world.GetComponent<Transform2D>(player);
 *     if (t) {
 *         t->Translate(10.0f, 0.0f);
 *     }
 * 
 *     // Create and add systems
 *     world.AddSystem(std::make_unique<RenderSystem2D>());
 * 
 *     // Update (in game loop)
 *     world.Update(deltaTime);
 * 
 * Architecture:
 * - Entity: Lightweight ID representing a game object
 * - Component: Pure data (e.g., Transform2D, Sprite2D, Renderable2D)
 * - System: Logic that processes entities with specific components
 * - World: Container managing all entities, components, and systems
 */

// Core ECS
#include "Entity.h"
#include "Component.h"
#include "System.h"
#include "World.h"
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
inline Entity CreateEntity2D(World& world, float x = 0.0f, float y = 0.0f) {
    Entity entity = world.CreateEntity();
    world.AddComponent(entity, std::make_unique<Transform2D>(x, y));
    return entity;
}

/**
 * Helper function to create a sprite entity with all common 2D components
 */
inline Entity CreateSpriteEntity(World& world, const std::string& texturePath, 
                                  float x = 0.0f, float y = 0.0f,
                                  float width = 0.0f, float height = 0.0f) {
    Entity entity = world.CreateEntity();
    
    // Add Transform2D
    world.AddComponent(entity, std::make_unique<Transform2D>(x, y));
    
    // Add Sprite2D
    auto sprite = std::make_unique<Sprite2D>(texturePath);
    if (width > 0.0f && height > 0.0f) {
        sprite->SetSize(width, height);
    }
    world.AddComponent(entity, std::move(sprite));
    
    // Add Renderable2D
    world.AddComponent(entity, std::make_unique<Renderable2D>(true, 0));
    
    return entity;
}

} // namespace ecs
} // namespace ului
