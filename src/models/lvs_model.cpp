#include "lvs_model.hpp"
#include "../ADDONS/cp_color.hpp"

//std
#include <iostream>
#include <cassert>

namespace lvs {

LvsModel::LvsModel(LvsDevice &device, const std::vector<Vertex> &vertices) : lvsDevice{device} {
  createVertexBuffers(vertices);
}

LvsModel::~LvsModel() {
  vkUnmapMemory(lvsDevice.device(), vertexBufferMemory);
  vkDestroyBuffer(lvsDevice.device(), vertexBuffer, nullptr);
  vkFreeMemory(lvsDevice.device(), vertexBufferMemory, nullptr);
}

void LvsModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());

  std::cout << cpc::Cyan << "Vertex count is: " << vertexCount << cpc::Reset << std::endl;
  assert(vertexCount >= 3 && "Vertex count must be at least 3");

  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
  lvsDevice.createBuffer(
    bufferSize,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    vertexBuffer,
    vertexBufferMemory
  );

  // Mapped once and left mapped (memory is host-coherent, so no explicit flush is needed) so
  // updateVertices() can overwrite it later with a plain memcpy instead of a map/unmap round trip.
  vkMapMemory(lvsDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &mappedVertexMemory);
  memcpy(mappedVertexMemory, vertices.data(), static_cast<size_t>(bufferSize));

  cachedVertices = vertices;
}

bool LvsModel::verticesEqual(const std::vector<Vertex> &a, const std::vector<Vertex> &b) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); i++) {
    if (a[i].position != b[i].position || a[i].color != b[i].color || a[i].uv != b[i].uv) return false;
  }
  return true;
}

bool LvsModel::updateVertices(const std::vector<Vertex> &vertices) {
  if (verticesEqual(vertices, cachedVertices)) return false; // unchanged - skip all GPU work

  if (vertices.size() != vertexCount) {
    // Count changed: the existing buffer is the wrong size, so it must be reallocated.
    vkUnmapMemory(lvsDevice.device(), vertexBufferMemory);
    vkDestroyBuffer(lvsDevice.device(), vertexBuffer, nullptr);
    vkFreeMemory(lvsDevice.device(), vertexBufferMemory, nullptr);
    createVertexBuffers(vertices); // also refreshes cachedVertices
    return true;
  }

  // Same count: reuse the existing persistently-mapped buffer, no reallocation needed.
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
  memcpy(mappedVertexMemory, vertices.data(), static_cast<size_t>(bufferSize));
  cachedVertices = vertices;
  return true;
}

void LvsModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void LvsModel::draw(VkCommandBuffer commandBuffer) {
  vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}

std::vector<VkVertexInputBindingDescription> LvsModel::Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> LvsModel::Vertex::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);
  // POSITION:
  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(Vertex, position);
  
  // COLOR:
  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(Vertex, color);

  attributeDescriptions[2].binding = 0;
  attributeDescriptions[2].location = 2;
  attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions[2].offset = offsetof(Vertex, uv); 
  
  return attributeDescriptions;
}

} // namespace lvs 