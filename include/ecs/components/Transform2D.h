#pragma once

#include "../Component.h"
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
 * Coordinate system:
 * - Position: (x, y) in pixels or world units
 * - Rotation: angle in radians (positive = counter-clockwise)
 * - Scale: (x, y) multipliers (1.0 = normal size)
 */
struct Transform2D : public Component {
    // Position in 2D space
    float x = 0.0f;
    float y = 0.0f;
    
    // Rotation in radians
    float rotation = 0.0f;
    
    // Scale factors
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    
    /**
     * Default constructor - creates identity transform at origin
     */
    Transform2D() = default;
    
    /**
     * Constructor with position
     */
    Transform2D(float posX, float posY)
        : x(posX), y(posY) {}
    
    /**
     * Constructor with position, rotation, and scale
     */
    Transform2D(float posX, float posY, float rot, float sclX, float sclY)
        : x(posX), y(posY), rotation(rot), scaleX(sclX), scaleY(sclY) {}
    
    /**
     * Set position
     */
    void SetPosition(float posX, float posY) {
        x = posX;
        y = posY;
    }
    
    /**
     * Set rotation in radians
     */
    void SetRotation(float rot) {
        rotation = rot;
    }
    
    /**
     * Set rotation in degrees
     */
    void SetRotationDegrees(float degrees) {
        rotation = degrees * std::numbers::pi_v<float> / 180.0f;
    }
    
    /**
     * Set uniform scale
     */
    void SetScale(float scale) {
        scaleX = scale;
        scaleY = scale;
    }
    
    /**
     * Set non-uniform scale
     */
    void SetScale(float sclX, float sclY) {
        scaleX = sclX;
        scaleY = sclY;
    }
    
    /**
     * Translate by offset
     */
    void Translate(float dx, float dy) {
        x += dx;
        y += dy;
    }
    
    /**
     * Rotate by angle in radians
     */
    void Rotate(float angle) {
        rotation += angle;
    }
    
    std::type_index GetTypeIndex() const override {
        return GetComponentTypeIndex<Transform2D>();
    }
};

} // namespace ecs
} // namespace ului
