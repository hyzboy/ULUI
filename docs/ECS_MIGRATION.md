# ECS Migration Guide / ECS 迁移指南

## 概述 / Overview

本指南帮助将现有代码迁移到新的 ECS (Entity Component System) 架构。

This guide helps migrate existing code to the new ECS (Entity Component System) architecture.

## 迁移策略 / Migration Strategy

### 第一阶段：逐步迁移 / Phase 1: Gradual Migration

ECS 系统与现有代码完全兼容，可以逐步迁移：

The ECS system is fully compatible with existing code, allowing gradual migration:

```cpp
class GameManager : public ului::Object {
public:
    GameManager() : Object("GameManager") {
        LogI("Initializing with ECS support");
    }
    
    void Initialize() {
        // Existing code continues to work
        InitializeOldSystem();
        
        // New ECS code
        m_world = std::make_unique<ecs::Scene>();
        m_world->AddSystem(std::make_unique<RenderSystem2D>());
    }
    
    void Update(float deltaTime) {
        // Mix old and new
        UpdateOldEntities(deltaTime);
        m_world->Update(deltaTime);
    }
    
private:
    std::unique_ptr<ecs::Scene> m_world;
};
```

### 第二阶段：识别迁移候选 / Phase 2: Identify Migration Candidates

优先迁移以下类型的对象：

Prioritize migrating these types of objects:

1. **游戏实体** / Game Entities
   - 玩家、敌人、道具等
   - Player, enemies, items, etc.

2. **UI 元素** / UI Elements
   - 按钮、标签、面板
   - Buttons, labels, panels

3. **粒子和效果** / Particles and Effects
   - 特效、动画对象
   - Effects, animated objects

### 第三阶段：转换模式 / Phase 3: Conversion Patterns

#### 模式 1：从类到组件 / Pattern 1: From Class to Components

**迁移前 / Before:**
```cpp
class Player {
    float x, y;
    float rotation;
    Texture* texture;
    bool visible;
    
    void Update(float dt) {
        x += velocity * dt;
        // ...
    }
    
    void Render() {
        // Render code
    }
};
```

**迁移后 / After:**
```cpp
// Create entity
Entity player = scene.CreateEntity();

// Add components (data only)
scene.AddComponent(player, std::make_unique<Transform2D>(x, y));

auto sprite = std::make_unique<Sprite2D>(texturePath);
scene.AddComponent(player, std::move(sprite));

scene.AddComponent(player, std::make_unique<Renderable2D>(true, 1));

auto velocity = std::make_unique<Velocity2D>(velocityX, 0);
scene.AddComponent(player, std::move(velocity));

// Logic moves to systems
class PlayerSystem : public System {
    void Update(float dt) override {
        auto entities = m_world->GetEntitiesWithComponent<Velocity2D>();
        for (Entity e : entities) {
            // Process entities
        }
    }
};
```

#### 模式 2：从继承到组合 / Pattern 2: From Inheritance to Composition

**迁移前 / Before:**
```cpp
class GameObject {
    // Base properties
};

class Enemy : public GameObject {
    // Enemy specific
};

class FlyingEnemy : public Enemy {
    // Flying specific
};
```

**迁移后 / After:**
```cpp
// Base entity with common components
Entity enemy = CreateSpriteEntity(scene, "enemy.png", x, y);

// Flying behavior through components
scene.AddComponent(enemy, std::make_unique<Velocity2D>(vx, vy));
scene.AddComponent(enemy, std::make_unique<FlyingBehavior>());

// Systems handle different behaviors
class FlyingSystem : public System {
    void Update(float dt) override {
        // Process flying entities
    }
};
```

## 迁移清单 / Migration Checklist

- [ ] 审计现有代码库
- [ ] 识别游戏对象和其属性
- [ ] 为每种属性创建组件
- [ ] 创建系统处理逻辑
- [ ] 逐个迁移对象
- [ ] 测试每次迁移
- [ ] 删除旧代码

- [ ] Audit existing codebase
- [ ] Identify game objects and their properties
- [ ] Create components for each property type
- [ ] Create systems to handle logic
- [ ] Migrate objects one by one
- [ ] Test each migration
- [ ] Remove old code

## 示例：UI 按钮迁移 / Example: UI Button Migration

### 迁移前 / Before

```cpp
class Button {
    float x, y, width, height;
    std::string text;
    bool hovered;
    bool pressed;
    std::function<void()> onClick;
    
    void Update() {
        // Update hover state
        // Check click
    }
    
    void Render() {
        // Draw button
    }
};
```

### 迁移后 / After

```cpp
// Components
struct UIButton : public Component {
    std::string text;
    std::function<void()> onClick;
    
    std::type_index GetTypeIndex() const override {
        return GetComponentTypeIndex<UIButton>();
    }
};

struct UIInteractive : public Component {
    bool hovered = false;
    bool pressed = false;
    
    std::type_index GetTypeIndex() const override {
        return GetComponentTypeIndex<UIInteractive>();
    }
};

struct Bounds2D : public Component {
    float width = 0.0f;
    float height = 0.0f;
    
    std::type_index GetTypeIndex() const override {
        return GetComponentTypeIndex<Bounds2D>();
    }
};

// System
class UIInputSystem : public System {
    void Update(float dt) override {
        auto entities = m_world->GetEntitiesWithComponent<UIButton>();
        for (Entity e : entities) {
            auto* transform = m_world->GetComponent<Transform2D>(e);
            auto* bounds = m_world->GetComponent<Bounds2D>(e);
            auto* interactive = m_world->GetComponent<UIInteractive>(e);
            auto* button = m_world->GetComponent<UIButton>(e);
            
            if (transform && bounds && interactive && button) {
                // Check hover
                interactive->hovered = CheckHover(transform, bounds);
                
                // Check click
                if (interactive->pressed && button->onClick) {
                    button->onClick();
                }
            }
        }
    }
};

// Usage
Entity button = scene.CreateEntity();
scene.AddComponent(button, std::make_unique<Transform2D>(100, 100));
scene.AddComponent(button, std::make_unique<Bounds2D>(200, 50));
scene.AddComponent(button, std::make_unique<UIInteractive>());

auto uiButton = std::make_unique<UIButton>();
uiButton->text = "Click Me";
uiButton->onClick = []() { std::cout << "Clicked!" << std::endl; };
scene.AddComponent(button, std::move(uiButton));

scene.AddComponent(button, std::make_unique<Renderable2D>(true, 100));
```

## 性能考虑 / Performance Considerations

1. **批处理 / Batching**: 按组件类型批量处理实体
2. **缓存 / Caching**: 系统可以缓存常用查询
3. **内存局部性 / Memory Locality**: 未来可以优化为数组存储

1. **Batching**: Process entities in batches by component type
2. **Caching**: Systems can cache frequently used queries
3. **Memory Locality**: Can be optimized to array storage in future

## 常见问题 / Common Issues

### Q: 如何处理实体间通信？ / How to handle inter-entity communication?

**A**: 使用组件和事件系统
```cpp
struct MessageReceiver : public Component {
    std::vector<Message> messages;
};

class MessageSystem : public System {
    void Update(float dt) override {
        // Deliver messages between entities
    }
};
```

### Q: 如何保存/加载实体？ / How to save/load entities?

**A**: 序列化组件数据
```cpp
// Future: Add serialization support
struct SaveData {
    Entity entity;
    std::vector<SerializedComponent> components;
};
```

## 下一步 / Next Steps

1. 为项目特定需求创建自定义组件
2. 实现专用系统
3. 开始逐步迁移最简单的对象
4. 添加工具函数简化常见操作
5. 考虑性能优化

1. Create custom components for project-specific needs
2. Implement specialized systems
3. Begin gradual migration with simplest objects
4. Add utility functions to simplify common operations
5. Consider performance optimizations

## 参考 / References

- [ECS.md](./ECS.md) - 完整 ECS 文档 / Complete ECS documentation
- [examples/ecs_example.cpp](../examples/ecs_example.cpp) - 工作示例 / Working example
