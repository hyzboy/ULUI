#pragma once

#include "../Component.h"
#include "../TransformDataStorage2D.h"
#include <cmath>
#include <numbers>

namespace ului {
namespace ecs {

/**
 * Transform2D component represents position, rotation, and scale in 2D space.
 * 
 * This component uses a 2D suffix to distinguish it from potential future
 * 3D transform components while keeping the API clear for 2D operations.
 * 
 * This component uses SOA (Structure of Arrays) pattern via TransformDataStorage2D
 * for better cache performance. The component itself only stores an index to the
 * actual data stored in TransformDataStorage2D.
 * 
 * Coordinate system:
 * - Position: (x, y) in pixels or world units
 * - Rotation: angle in radians (positive = counter-clockwise)
 * - Scale: (x, y) multipliers (1.0 = normal size)
 */
struct Transform2D : public Component {
    // Index to data in TransformDataStorage2D
    uint32_t dataIndex = 0;
    
    // Pointer to the shared storage (managed by World/ComponentManager)
    TransformDataStorage2D* storage = nullptr;
    
    /**
     * Default constructor - creates identity transform at origin
     */
    Transform2D() = default;
    
    /**
     * Constructor with storage and position
     */
    Transform2D(TransformDataStorage2D* stor, float posX = 0.0f, float posY = 0.0f)
        : storage(stor) {
        if (storage) {
            dataIndex = storage->Allocate(posX, posY);
        }
    }
    
    /**
     * Constructor with storage, position, rotation, and scale
     */
    Transform2D(TransformDataStorage2D* stor, float posX, float posY, float rot, float sclX, float sclY)
        : storage(stor) {
        if (storage) {
            dataIndex = storage->Allocate(posX, posY, rot, sclX, sclY);
        }
    }
    
    /**
     * Destructor - free the data slot
     */
    ~Transform2D() {
        if (storage) {
            storage->Free(dataIndex);
        }
    }
    
    // Position accessors
    float GetX() const { return storage ? storage->GetX(dataIndex) : 0.0f; }
    float GetY() const { return storage ? storage->GetY(dataIndex) : 0.0f; }
    void SetX(float value) { if (storage) storage->SetX(dataIndex, value); }
    void SetY(float value) { if (storage) storage->SetY(dataIndex, value); }
    
    /**
     * Set position
     */
    void SetPosition(float posX, float posY) {
        if (storage) {
            storage->SetPosition(dataIndex, posX, posY);
        }
    }
    
    /**
     * Get rotation in radians
     */
    float GetRotation() const { 
        return storage ? storage->GetRotation(dataIndex) : 0.0f; 
    }
    
    /**
     * Set rotation in radians
     */
    void SetRotation(float rot) {
        if (storage) {
            storage->SetRotation(dataIndex, rot);
        }
    }
    
    /**
     * Set rotation in degrees
     */
    void SetRotationDegrees(float degrees) {
        SetRotation(degrees * std::numbers::pi_v<float> / 180.0f);
    }
    
    /**
     * Get scale X
     */
    float GetScaleX() const {
        return storage ? storage->GetScaleX(dataIndex) : 1.0f;
    }
    
    /**
     * Get scale Y
     */
    float GetScaleY() const {
        return storage ? storage->GetScaleY(dataIndex) : 1.0f;
    }
    
    /**
     * Set uniform scale
     */
    void SetScale(float scale) {
        if (storage) {
            storage->SetScale(dataIndex, scale);
        }
    }
    
    /**
     * Set non-uniform scale
     */
    void SetScale(float sclX, float sclY) {
        if (storage) {
            storage->SetScale(dataIndex, sclX, sclY);
        }
    }
    
    /**
     * Translate by offset
     */
    void Translate(float dx, float dy) {
        if (storage) {
            storage->SetPosition(dataIndex, 
                                storage->GetX(dataIndex) + dx,
                                storage->GetY(dataIndex) + dy);
        }
    }
    
    /**
     * Rotate by angle in radians
     */
    void Rotate(float angle) {
        if (storage) {
            storage->SetRotation(dataIndex, storage->GetRotation(dataIndex) + angle);
        }
    }
    
    // Legacy compatibility properties (for backward compatibility with existing code)
    // These return references to temporary values - use getter/setter methods instead
    [[deprecated("Use GetX()/SetX() instead")]]
    float& x() { 
        static float temp = GetX(); 
        temp = GetX(); 
        return temp; 
    }
    
    [[deprecated("Use GetY()/SetY() instead")]]
    float& y() { 
        static float temp = GetY(); 
        temp = GetY(); 
        return temp; 
    }
    
    [[deprecated("Use GetRotation()/SetRotation() instead")]]
    float& rotation() { 
        static float temp = GetRotation(); 
        temp = GetRotation(); 
        return temp; 
    }
    
    [[deprecated("Use GetScaleX()/SetScaleX() instead")]]
    float& scaleX() { 
        static float temp = GetScaleX(); 
        temp = GetScaleX(); 
        return temp; 
    }
    
    [[deprecated("Use GetScaleY()/SetScaleY() instead")]]
    float& scaleY() { 
        static float temp = GetScaleY(); 
        temp = GetScaleY(); 
        return temp; 
    }
    
    std::type_index GetTypeIndex() const override {
        return GetComponentTypeIndex<Transform2D>();
    }
};

} // namespace ecs
} // namespace ului
