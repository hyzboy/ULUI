/**
 * ULUI ECS Example
 * 
 * This example demonstrates basic usage of the ECS (Entity Component System)
 * for 2D applications. It shows how to:
 * - Create entities
 * - Add components
 * - Create and use systems
 * - Query and process entities
 */

#include "ecs/ECS.h"
#include <iostream>
#include <cmath>

using namespace ului::ecs;

// Custom component: Velocity for movement
struct Velocity2D : public Component {
    float vx = 0.0f;
    float vy = 0.0f;
    
    Velocity2D() = default;
    Velocity2D(float x, float y) : vx(x), vy(y) {}
    
    std::type_index GetTypeIndex() const override {
        return GetComponentTypeIndex<Velocity2D>();
    }
};

// Custom system: Movement system applies velocity to transform
class MovementSystem : public System {
public:
    void Update(float deltaTime) override {
        // Get all entities with Transform2D component
        auto entities = m_scene->GetEntitiesWithComponent<Transform2D>();
        
        for (Entity entity : entities) {
            // Check if entity also has Velocity2D
            if (m_scene->HasComponent<Velocity2D>(entity)) {
                Transform2D* transform = m_scene->GetComponent<Transform2D>(entity);
                Velocity2D* velocity = m_scene->GetComponent<Velocity2D>(entity);
                
                if (transform && velocity) {
                    // Apply velocity
                    transform->Translate(velocity->vx * deltaTime, 
                                        velocity->vy * deltaTime);
                }
            }
        }
    }
};

// Custom system: Render system (simplified example)
class RenderSystem : public System {
public:
    void Update(float deltaTime) override {
        // Get all entities with both Transform2D and Sprite2D
        auto entities = m_scene->GetEntitiesWithComponent<Transform2D>();
        
        std::cout << "=== Rendering Frame ===" << std::endl;
        
        for (Entity entity : entities) {
            if (m_scene->HasComponent<Sprite2D>(entity) &&
                m_scene->HasComponent<Renderable2D>(entity)) {
                
                Transform2D* transform = m_scene->GetComponent<Transform2D>(entity);
                Sprite2D* sprite = m_scene->GetComponent<Sprite2D>(entity);
                Renderable2D* renderable = m_scene->GetComponent<Renderable2D>(entity);
                
                if (transform && sprite && renderable && renderable->visible) {
                    std::cout << "Entity " << entity 
                              << " at (" << transform->GetX() << ", " << transform->GetY() << ")"
                              << " rotation: " << transform->GetRotation()
                              << " texture: " << sprite->texturePath
                              << " layer: " << renderable->layer
                              << std::endl;
                }
            }
        }
    }
};

// Example usage
int main() {
    std::cout << "ULUI ECS Example - 2D Entity Component System" << std::endl;
    std::cout << "=============================================" << std::endl << std::endl;
    
    // Create the ECS scene
    Scene scene;
    
    // Add systems to the scene
    scene.AddSystem(std::make_unique<MovementSystem>());
    scene.AddSystem(std::make_unique<RenderSystem>());
    
    std::cout << "1. Creating entities..." << std::endl;
    
    // Create a player entity using helper function
    Entity player = CreateSpriteEntity(scene, "player.png", 100.0f, 100.0f, 64.0f, 64.0f);
    
    // Add velocity to player
    auto playerVelocity = std::make_unique<Velocity2D>(50.0f, 0.0f);  // Moving right
    scene.AddComponent(player, std::move(playerVelocity));
    
    // Customize player renderable
    Renderable2D* playerRenderable = scene.GetComponent<Renderable2D>(player);
    if (playerRenderable) {
        playerRenderable->SetLayer(10);
        playerRenderable->SetTint(255, 200, 200);  // Slight red tint
    }
    
    std::cout << "   Created player entity (ID: " << player << ")" << std::endl;
    
    // Create an enemy entity manually
    Entity enemy = scene.CreateEntity();
    scene.AddComponent(enemy, std::make_unique<Transform2D>(&scene.GetTransformStorage2D(), 300.0f, 150.0f));
    
    auto enemySprite = std::make_unique<Sprite2D>("enemy.png");
    enemySprite->SetSize(48.0f, 48.0f);
    enemySprite->SetCenterPivot();
    scene.AddComponent(enemy, std::move(enemySprite));
    
    scene.AddComponent(enemy, std::make_unique<Renderable2D>(true, 5));
    scene.AddComponent(enemy, std::make_unique<Velocity2D>(-30.0f, 20.0f));  // Moving left and down
    
    std::cout << "   Created enemy entity (ID: " << enemy << ")" << std::endl;
    
    // Create a static background entity (no velocity)
    Entity background = CreateSpriteEntity(scene, "background.png", 0.0f, 0.0f, 800.0f, 600.0f);
    Renderable2D* bgRenderable = scene.GetComponent<Renderable2D>(background);
    if (bgRenderable) {
        bgRenderable->SetLayer(0);  // Behind everything
    }
    
    std::cout << "   Created background entity (ID: " << background << ")" << std::endl;
    std::cout << std::endl;
    
    // Simulate a few frames
    std::cout << "2. Simulating game loop..." << std::endl << std::endl;
    
    float deltaTime = 0.016f;  // ~60 FPS
    
    for (int frame = 0; frame < 3; frame++) {
        std::cout << "Frame " << frame << " (dt=" << deltaTime << "s):" << std::endl;
        scene.Update(deltaTime);
        std::cout << std::endl;
    }
    
    // Demonstrate component manipulation
    std::cout << "3. Modifying entities..." << std::endl;
    
    // Rotate player
    Transform2D* playerTransform = scene.GetComponent<Transform2D>(player);
    if (playerTransform) {
        playerTransform->SetRotationDegrees(45.0f);
        std::cout << "   Rotated player 45 degrees" << std::endl;
    }
    
    // Stop enemy movement
    scene.RemoveComponent<Velocity2D>(enemy);
    std::cout << "   Removed enemy velocity (stopped)" << std::endl;
    
    // Hide background
    Renderable2D* bgRend = scene.GetComponent<Renderable2D>(background);
    if (bgRend) {
        bgRend->SetVisible(false);
        std::cout << "   Hidden background" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "4. Rendering after modifications..." << std::endl;
    scene.Update(deltaTime);
    std::cout << std::endl;
    
    // Query entities
    std::cout << "5. Querying entities..." << std::endl;
    auto allEntities = scene.GetAllEntities();
    std::cout << "   Total entities: " << allEntities.size() << std::endl;
    
    auto entitiesWithTransform = scene.GetEntitiesWithComponent<Transform2D>();
    std::cout << "   Entities with Transform2D: " << entitiesWithTransform.size() << std::endl;
    
    auto entitiesWithVelocity = scene.GetEntitiesWithComponent<Velocity2D>();
    std::cout << "   Entities with Velocity2D: " << entitiesWithVelocity.size() << std::endl;
    
    std::cout << std::endl;
    
    // Cleanup
    std::cout << "6. Destroying entities..." << std::endl;
    scene.DestroyEntity(enemy);
    std::cout << "   Destroyed enemy entity" << std::endl;
    std::cout << "   Remaining entities: " << scene.GetAllEntities().size() << std::endl;
    
    std::cout << std::endl;
    std::cout << "ECS example completed successfully!" << std::endl;
    
    return 0;
}
