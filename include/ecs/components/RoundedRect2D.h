#pragma once

#include "../Component.h"
#include <cstdint>

namespace ului {
namespace ecs {

/**
 * RoundedRect2D component for rendering rounded rectangles.
 * Stores the rectangle dimensions and corner radius.
 */
struct RoundedRect2D : public Component {
    // Rectangle dimensions
    float width = 100.0f;
    float height = 100.0f;
    
    // Corner radius (same for all corners)
    float cornerRadius = 10.0f;
    
    // Color (RGBA, 0-1 range)
    float colorR = 1.0f;
    float colorG = 1.0f;
    float colorB = 1.0f;
    float colorA = 1.0f;
    
    /**
     * Default constructor
     */
    RoundedRect2D() = default;
    
    /**
     * Constructor with dimensions
     */
    RoundedRect2D(float w, float h, float radius = 10.0f)
        : width(w), height(h), cornerRadius(radius) {}
    
    /**
     * Constructor with dimensions and color
     */
    RoundedRect2D(float w, float h, float radius, float r, float g, float b, float a = 1.0f)
        : width(w), height(h), cornerRadius(radius), colorR(r), colorG(g), colorB(b), colorA(a) {}
    
    /**
     * Set size
     */
    void SetSize(float w, float h) {
        width = w;
        height = h;
    }
    
    /**
     * Set corner radius
     */
    void SetCornerRadius(float radius) {
        cornerRadius = radius;
    }
    
    /**
     * Set color
     */
    void SetColor(float r, float g, float b, float a = 1.0f) {
        colorR = r;
        colorG = g;
        colorB = b;
        colorA = a;
    }
    
    std::type_index GetTypeIndex() const override {
        return GetComponentTypeIndex<RoundedRect2D>();
    }
};

} // namespace ecs
} // namespace ului
