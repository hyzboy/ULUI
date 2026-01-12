# ULUI ECS (Entity Component System) - 2D

## 概述 / Overview

ULUI 的 ECS 系统是专为 2D 应用设计的实体组件系统架构。

The ULUI ECS system is an Entity Component System architecture specifically designed for 2D applications.

## 为什么使用 2D 后缀？ / Why the 2D Suffix?

组件名称使用 2D 后缀（如 `Transform2D`, `Sprite2D`）以：
1. 明确表示这是 2D 专用组件
2. 为未来可能的 3D 组件预留命名空间
3. 避免与其他库的命名冲突

Component names use the 2D suffix (e.g., `Transform2D`, `Sprite2D`) to:
1. Clearly indicate these are 2D-specific components
2. Reserve namespace for possible future 3D components
3. Avoid naming conflicts with other libraries

## 核心概念 / Core Concepts

### Entity (实体)
轻量级的唯一标识符，代表游戏中的一个对象。

A lightweight unique identifier representing an object in the game.

```cpp
Entity player = world.CreateEntity();
```

### Component (组件)
纯数据容器，附加到实体上描述其属性。

Pure data containers attached to entities to describe their properties.

```cpp
struct Transform2D : public Component {
    float x, y;           // Position
    float rotation;       // Rotation in radians
    float scaleX, scaleY; // Scale factors
};
```

### System (系统)
包含逻辑的类，处理具有特定组件的实体。

Classes containing logic that process entities with specific components.

```cpp
class RenderSystem : public System {
    void Update(float deltaTime) override {
        // Process all entities with Transform2D and Sprite2D
    }
};
```

### World (世界)
管理所有实体、组件和系统的容器。

Container managing all entities, components, and systems.

```cpp
World world;
world.CreateEntity();
world.AddSystem(std::make_unique<RenderSystem>());
world.Update(deltaTime);
```

## 2D 组件 / 2D Components

### Transform2D
位置、旋转和缩放变换。

Position, rotation, and scale transformation.

```cpp
auto transform = std::make_unique<Transform2D>(100.0f, 200.0f);
transform->SetRotation(1.57f);  // 90 degrees in radians
transform->SetScale(2.0f);       // Uniform scale
world.AddComponent(entity, std::move(transform));
```

### Sprite2D
精灵渲染数据（纹理、源矩形、大小）。

Sprite rendering data (texture, source rectangle, size).

```cpp
auto sprite = std::make_unique<Sprite2D>("player.png");
sprite->SetSize(64.0f, 64.0f);
sprite->SetCenterPivot();
sprite->SetFlip(true, false);  // Flip horizontally
world.AddComponent(entity, std::move(sprite));
```

### Renderable2D
渲染属性（可见性、层级、不透明度、着色）。

Rendering properties (visibility, layer, opacity, tint).

```cpp
auto renderable = std::make_unique<Renderable2D>(true, 1);
renderable->SetOpacity(0.8f);
renderable->SetTint(255, 128, 128);  // Red tint
world.AddComponent(entity, std::move(renderable));
```

## 使用示例 / Usage Example

### 创建精灵实体 / Creating a Sprite Entity

```cpp
#include "ecs/ECS.h"
using namespace ului::ecs;

// Create world
World world;

// Method 1: Manual creation
Entity player = world.CreateEntity();
world.AddComponent(player, std::make_unique<Transform2D>(100.0f, 200.0f));
world.AddComponent(player, std::make_unique<Sprite2D>("player.png"));
world.AddComponent(player, std::make_unique<Renderable2D>(true, 1));

// Method 2: Using helper function
Entity enemy = CreateSpriteEntity(world, "enemy.png", 300.0f, 200.0f, 64.0f, 64.0f);
```

### 访问和修改组件 / Accessing and Modifying Components

```cpp
// Get component
Transform2D* transform = world.GetComponent<Transform2D>(player);
if (transform) {
    transform->Translate(10.0f, 0.0f);  // Move right
    transform->Rotate(0.1f);             // Rotate
}

// Check if entity has component
if (world.HasComponent<Sprite2D>(player)) {
    Sprite2D* sprite = world.GetComponent<Sprite2D>(player);
    sprite->SetFlip(true, false);
}

// Remove component
world.RemoveComponent<Renderable2D>(player);
```

### 创建自定义系统 / Creating Custom Systems

```cpp
class MovementSystem : public System {
public:
    void Update(float deltaTime) override {
        // Get all entities with Transform2D
        auto entities = m_world->GetEntitiesWithComponent<Transform2D>();
        
        for (Entity entity : entities) {
            Transform2D* transform = m_world->GetComponent<Transform2D>(entity);
            if (transform) {
                // Apply movement logic
                transform->Translate(speed * deltaTime, 0.0f);
            }
        }
    }
    
private:
    float speed = 100.0f;
};

// Add system to world
world.AddSystem(std::make_unique<MovementSystem>());
```

### 游戏循环 / Game Loop

```cpp
World world;

// Setup
Entity player = CreateSpriteEntity(world, "player.png", 100, 100, 64, 64);
world.AddSystem(std::make_unique<MovementSystem>());
world.AddSystem(std::make_unique<RenderSystem>());

// Game loop
float deltaTime = 0.016f;  // ~60 FPS
while (running) {
    world.Update(deltaTime);
}
```

## 创建自定义组件 / Creating Custom Components

```cpp
struct Velocity2D : public Component {
    float vx = 0.0f;
    float vy = 0.0f;
    
    Velocity2D() = default;
    Velocity2D(float x, float y) : vx(x), vy(y) {}
    
    std::type_index GetTypeIndex() const override {
        return GetComponentTypeIndex<Velocity2D>();
    }
};

// Usage
auto velocity = std::make_unique<Velocity2D>(50.0f, 0.0f);
world.AddComponent(entity, std::move(velocity));
```

## 查询实体 / Querying Entities

```cpp
// Get all entities with Transform2D
auto entities = world.GetEntitiesWithComponent<Transform2D>();

// Check multiple components
for (Entity entity : world.GetAllEntities()) {
    if (world.HasComponent<Transform2D>(entity) && 
        world.HasComponent<Sprite2D>(entity)) {
        // Process entity
    }
}
```

## 架构设计原则 / Architecture Design Principles

1. **组件只存储数据** / Components Only Store Data
   - 不要在组件中放逻辑
   - 保持组件简单和可序列化
   
   Don't put logic in components
   Keep components simple and serializable

2. **系统包含逻辑** / Systems Contain Logic
   - 所有游戏逻辑都在系统中
   - 系统是无状态的（状态在组件中）
   
   All game logic goes in systems
   Systems are stateless (state is in components)

3. **实体只是 ID** / Entities Are Just IDs
   - 实体不存储数据
   - 实体通过组件组合定义
   
   Entities don't store data
   Entities are defined by component composition

4. **使用组合而非继承** / Use Composition Over Inheritance
   - 通过添加组件来扩展功能
   - 避免深层继承层次
   
   Extend functionality by adding components
   Avoid deep inheritance hierarchies

## 性能注意事项 / Performance Considerations

当前实现使用基于 map 的简单方法，适合中小规模应用。

Current implementation uses a simple map-based approach suitable for small to medium scale applications.

未来优化方向：
- 使用对象池减少内存分配
- 使用数组存储提高缓存局部性
- 添加组件缓存和批处理

Future optimization directions:
- Use object pools to reduce memory allocation
- Use array storage for better cache locality
- Add component caching and batching

## 与现有代码集成 / Integration with Existing Code

ECS 系统与现有的 `Object` 基类兼容：

The ECS system is compatible with the existing `Object` base class:

```cpp
class GameWorld : public ului::Object {
public:
    GameWorld() : Object("GameWorld") {
        LogI("Initializing game world");
    }
    
    void Update(float deltaTime) {
        m_ecsWorld.Update(deltaTime);
        LogD("Updated ECS world");
    }
    
private:
    ecs::World m_ecsWorld;
};
```

## 下一步 / Next Steps

1. 根据具体需求创建更多组件（碰撞、物理、动画等）
2. 实现特定于游戏的系统
3. 与现有渲染系统集成
4. 添加组件序列化支持

1. Create more components based on specific needs (collision, physics, animation, etc.)
2. Implement game-specific systems
3. Integrate with existing rendering system
4. Add component serialization support
