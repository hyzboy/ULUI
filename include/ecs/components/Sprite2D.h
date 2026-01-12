#pragma once

#include "../Component.h"
#include <string>

namespace ului {
namespace ecs {

/**
 * Sprite2D component represents a 2D sprite for rendering.
 * 
 * This component stores sprite-specific data like texture reference,
 * source rectangle, and pivot point for 2D sprite rendering.
 */
struct Sprite2D : public Component {
    // Texture ID or path (implementation-specific)
    std::string texturePath;
    uint32_t textureId = 0;
    
    // Source rectangle in texture (in pixels)
    // If all zeros, use entire texture
    float srcX = 0.0f;
    float srcY = 0.0f;
    float srcWidth = 0.0f;
    float srcHeight = 0.0f;
    
    // Display size in world units (if 0, use source size)
    float width = 0.0f;
    float height = 0.0f;
    
    // Pivot point (0-1 range, relative to sprite bounds)
    // (0, 0) = top-left, (0.5, 0.5) = center, (1, 1) = bottom-right
    float pivotX = 0.5f;
    float pivotY = 0.5f;
    
    // Flip flags
    bool flipX = false;
    bool flipY = false;
    
    /**
     * Default constructor
     */
    Sprite2D() = default;
    
    /**
     * Constructor with texture path
     */
    explicit Sprite2D(const std::string& texPath)
        : texturePath(texPath) {}
    
    /**
     * Constructor with texture ID
     */
    explicit Sprite2D(uint32_t texId)
        : textureId(texId) {}
    
    /**
     * Set texture by path
     */
    void SetTexture(const std::string& texPath) {
        texturePath = texPath;
        textureId = 0;
    }
    
    /**
     * Set texture by ID
     */
    void SetTexture(uint32_t texId) {
        textureId = texId;
        texturePath.clear();
    }
    
    /**
     * Set source rectangle in texture
     */
    void SetSourceRect(float x, float y, float w, float h) {
        srcX = x;
        srcY = y;
        srcWidth = w;
        srcHeight = h;
    }
    
    /**
     * Set display size
     */
    void SetSize(float w, float h) {
        width = w;
        height = h;
    }
    
    /**
     * Set pivot point (0-1 range)
     */
    void SetPivot(float x, float y) {
        pivotX = x;
        pivotY = y;
    }
    
    /**
     * Set center pivot
     */
    void SetCenterPivot() {
        pivotX = 0.5f;
        pivotY = 0.5f;
    }
    
    /**
     * Set flip flags
     */
    void SetFlip(bool horizontal, bool vertical) {
        flipX = horizontal;
        flipY = vertical;
    }
    
    std::type_index GetTypeIndex() const override {
        return GetComponentTypeIndex<Sprite2D>();
    }
};

} // namespace ecs
} // namespace ului
