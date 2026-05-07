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

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
  };

  LvsModel(LvsDevice &device, const std::vector<Vertex> &vertices);
  ~LvsModel();

  LvsModel(const LvsModel&) = delete;
  LvsModel& operator=(const LvsModel&) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);
private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);

  LvsDevice& lvsDevice;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  uint32_t vertexCount;
};

}