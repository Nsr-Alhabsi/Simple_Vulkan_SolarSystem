#pragma once

#include "../core/lvs_device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

// std
#include <vector>

namespace lvs {

class LvsModel {
public:
  struct Vertex {
    glm::vec2 position;
    glm::vec3 color;
    glm::vec2 uv;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
  };

  LvsModel(LvsDevice &device, const std::vector<Vertex> &vertices);
  ~LvsModel();

  LvsModel(const LvsModel&) = delete;
  LvsModel& operator=(const LvsModel&) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

  /**
   * @brief Overwrites this model's vertex data, e.g. for morph/skeletal-style mesh updates.
   *
   * Compares `vertices` against the last-uploaded data first: if nothing actually changed, this
   * is a no-op and returns false immediately — no GPU work happens. If the vertex count is
   * unchanged, the existing persistently-mapped host-coherent buffer is overwritten in place
   * (a plain memcpy — no buffer recreation, no staging buffer). Only a vertex-count change forces
   * the vertex buffer to be reallocated at the new size.
   *
   * @param vertices The new vertex data to upload.
   * @return true if the GPU buffer was actually updated, false if `vertices` was identical to
   *         what's already uploaded.
   */
  bool updateVertices(const std::vector<Vertex> &vertices);

private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  static bool verticesEqual(const std::vector<Vertex> &a, const std::vector<Vertex> &b);

  LvsDevice& lvsDevice;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  uint32_t vertexCount;

  // Left mapped for the lifetime of vertexBufferMemory (unmapped only in the destructor or right
  // before a resize-recreate) so updateVertices() never pays a map/unmap cost on the common path.
  void* mappedVertexMemory = nullptr;

  // CPU-side snapshot of what's currently uploaded, purely so updateVertices() can detect a no-op
  // call cheaply without touching the GPU at all.
  std::vector<Vertex> cachedVertices;
};

}