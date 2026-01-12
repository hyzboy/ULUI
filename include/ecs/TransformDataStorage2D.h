#pragma once

#include <vector>
#include <cstdint>

namespace ului {
namespace ecs {

/**
 * TransformDataStorage2D uses Structure of Arrays (SOA) pattern for better cache performance.
 * 
 * Instead of storing transform data in each Transform2D component (AoS pattern),
 * all transform data is stored in contiguous arrays here. Transform2D components
 * only store an index to reference their data.
 * 
 * Benefits:
 * - Better cache locality when iterating over transforms
 * - More efficient SIMD operations
 * - Reduced memory fragmentation
 */
class TransformDataStorage2D {
public:
    /**
     * Allocate a new transform data slot
     * @return Index to the allocated slot
     */
    uint32_t Allocate() {
        // Reuse a freed index if available
        if (!m_freeIndices.empty()) {
            uint32_t index = m_freeIndices.back();
            m_freeIndices.pop_back();
            
            // Reset to default values
            m_posX[index] = 0.0f;
            m_posY[index] = 0.0f;
            m_rotation[index] = 0.0f;
            m_scaleX[index] = 1.0f;
            m_scaleY[index] = 1.0f;
            
            return index;
        }
        
        uint32_t index = static_cast<uint32_t>(m_posX.size());
        
        // Add default values
        m_posX.push_back(0.0f);
        m_posY.push_back(0.0f);
        m_rotation.push_back(0.0f);
        m_scaleX.push_back(1.0f);
        m_scaleY.push_back(1.0f);
        
        return index;
    }
    
    /**
     * Allocate with initial values
     */
    uint32_t Allocate(float x, float y, float rot = 0.0f, float sclX = 1.0f, float sclY = 1.0f) {
        // Reuse a freed index if available
        if (!m_freeIndices.empty()) {
            uint32_t index = m_freeIndices.back();
            m_freeIndices.pop_back();
            
            // Set initial values
            m_posX[index] = x;
            m_posY[index] = y;
            m_rotation[index] = rot;
            m_scaleX[index] = sclX;
            m_scaleY[index] = sclY;
            
            return index;
        }
        
        uint32_t index = static_cast<uint32_t>(m_posX.size());
        
        m_posX.push_back(x);
        m_posY.push_back(y);
        m_rotation.push_back(rot);
        m_scaleX.push_back(sclX);
        m_scaleY.push_back(sclY);
        
        return index;
    }
    
    /**
     * Free a transform data slot (marks for reuse)
     * Note: For simplicity, we don't actually remove data, just mark as free
     */
    void Free(uint32_t index) {
        if (index < m_posX.size()) {
            m_freeIndices.push_back(index);
        }
    }
    
    // Position accessors
    float GetX(uint32_t index) const { 
        return index < m_posX.size() ? m_posX[index] : 0.0f; 
    }
    float GetY(uint32_t index) const { 
        return index < m_posY.size() ? m_posY[index] : 0.0f; 
    }
    void SetX(uint32_t index, float value) { 
        if (index < m_posX.size()) m_posX[index] = value; 
    }
    void SetY(uint32_t index, float value) { 
        if (index < m_posY.size()) m_posY[index] = value; 
    }
    void SetPosition(uint32_t index, float x, float y) {
        if (index < m_posX.size()) {
            m_posX[index] = x;
            m_posY[index] = y;
        }
    }
    
    // Rotation accessors
    float GetRotation(uint32_t index) const { 
        return index < m_rotation.size() ? m_rotation[index] : 0.0f; 
    }
    void SetRotation(uint32_t index, float value) { 
        if (index < m_rotation.size()) m_rotation[index] = value; 
    }
    
    // Scale accessors
    float GetScaleX(uint32_t index) const { 
        return index < m_scaleX.size() ? m_scaleX[index] : 1.0f; 
    }
    float GetScaleY(uint32_t index) const { 
        return index < m_scaleY.size() ? m_scaleY[index] : 1.0f; 
    }
    void SetScaleX(uint32_t index, float value) { 
        if (index < m_scaleX.size()) m_scaleX[index] = value; 
    }
    void SetScaleY(uint32_t index, float value) { 
        if (index < m_scaleY.size()) m_scaleY[index] = value; 
    }
    void SetScale(uint32_t index, float x, float y) {
        if (index < m_scaleX.size()) {
            m_scaleX[index] = x;
            m_scaleY[index] = y;
        }
    }
    void SetScale(uint32_t index, float value) {
        if (index < m_scaleX.size()) {
            m_scaleX[index] = value;
            m_scaleY[index] = value;
        }
    }
    
    // Batch operations for better performance
    const float* GetPositionXArray() const { return m_posX.data(); }
    const float* GetPositionYArray() const { return m_posY.data(); }
    const float* GetRotationArray() const { return m_rotation.data(); }
    const float* GetScaleXArray() const { return m_scaleX.data(); }
    const float* GetScaleYArray() const { return m_scaleY.data(); }
    
    float* GetPositionXArray() { return m_posX.data(); }
    float* GetPositionYArray() { return m_posY.data(); }
    float* GetRotationArray() { return m_rotation.data(); }
    float* GetScaleXArray() { return m_scaleX.data(); }
    float* GetScaleYArray() { return m_scaleY.data(); }
    
    /**
     * Get total number of allocated slots (including freed ones)
     */
    size_t GetCount() const { return m_posX.size(); }
    
    /**
     * Clear all data
     */
    void Clear() {
        m_posX.clear();
        m_posY.clear();
        m_rotation.clear();
        m_scaleX.clear();
        m_scaleY.clear();
        m_freeIndices.clear();
    }
    
private:
    // Structure of Arrays - separate arrays for each field
    std::vector<float> m_posX;
    std::vector<float> m_posY;
    std::vector<float> m_rotation;
    std::vector<float> m_scaleX;
    std::vector<float> m_scaleY;
    
    // Free list for reusing indices
    std::vector<uint32_t> m_freeIndices;
};

} // namespace ecs
} // namespace ului
