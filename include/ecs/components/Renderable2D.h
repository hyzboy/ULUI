#pragma once

#include "../Component.h"
#include <cstdint>

namespace ului {
namespace ecs {

/**
 * Renderable2D component contains rendering properties for 2D objects.
 * 
 * This component controls visibility, layer ordering, and blending
 * for 2D rendering operations.
 */
struct Renderable2D : public Component {
    // Visibility flag
    bool visible = true;
    
    // Rendering layer (higher values render on top)
    int32_t layer = 0;
    
    // Opacity (0.0 = fully transparent, 1.0 = fully opaque)
    float opacity = 1.0f;
    
    // Tint color (RGBA, 0-255)
    uint8_t tintR = 255;
    uint8_t tintG = 255;
    uint8_t tintB = 255;
    uint8_t tintA = 255;
    
    /**
     * Default constructor
     */
    Renderable2D() = default;
    
    /**
     * Constructor with visibility and layer
     */
    Renderable2D(bool vis, int32_t lyr = 0)
        : visible(vis), layer(lyr) {}
    
    /**
     * Set visibility
     */
    void SetVisible(bool vis) {
        visible = vis;
    }
    
    /**
     * Set rendering layer
     */
    void SetLayer(int32_t lyr) {
        layer = lyr;
    }
    
    /**
     * Set opacity (0.0 to 1.0)
     */
    void SetOpacity(float op) {
        opacity = op;
        if (opacity < 0.0f) opacity = 0.0f;
        if (opacity > 1.0f) opacity = 1.0f;
    }
    
    /**
     * Set tint color (RGB, 0-255)
     */
    void SetTint(uint8_t r, uint8_t g, uint8_t b) {
        tintR = r;
        tintG = g;
        tintB = b;
    }
    
    /**
     * Set tint color with alpha (RGBA, 0-255)
     */
    void SetTint(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        tintR = r;
        tintG = g;
        tintB = b;
        tintA = a;
    }
    
    /**
     * Set white tint (no color modification)
     */
    void ResetTint() {
        tintR = tintG = tintB = tintA = 255;
    }
    
    std::type_index GetTypeIndex() const override {
        return GetComponentTypeIndex<Renderable2D>();
    }
};

} // namespace ecs
} // namespace ului
