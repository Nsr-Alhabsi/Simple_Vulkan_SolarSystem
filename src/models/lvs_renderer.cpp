#include "../vulkan_simulation.hpp"
#include "../ADDONS/cp_color.hpp"
#include "../ADDONS/json_setup.hpp"
#include "lvs_renderer.hpp"

// std
#include <stdexcept>


namespace lvs { // lvs stands for large - vulkan - simulation

LvsRenderer::LvsRenderer(LvsWindow &window, LvsDevice &device) : lvsWindow{window}, lvsDevice{device} {
  recreateSwapChain();
  createCommandBuffers();
}

LvsRenderer::~LvsRenderer() { freeCommandBuffers(); }

void LvsRenderer::recreateSwapChain() {
  auto extent = lvsWindow.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = lvsWindow.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(lvsDevice.device());

  if (lvsSwapChain == nullptr) {
    lvsSwapChain = std::make_unique<LvsSwapChain>(lvsDevice, extent);
  } else {
    std::shared_ptr<LvsSwapChain> oldSwapChain = std::move(lvsSwapChain);
    lvsSwapChain = std::make_unique<LvsSwapChain>(lvsDevice, extent, oldSwapChain);

    auto details = oldSwapChain->compareSwapFormats(*lvsSwapChain.get());
    if (!details.allCompatible()) {
      if (!details.depthMatches) { std::runtime_error("Logic Error: Depth format changed"); }
      if (!details.colorMatches) { std::runtime_error("Logic Error: Color format changed"); }
    }
  }

  // TODO: Remember to create the pipeline in this line... <----- DO NOT FORGET
}

void LvsRenderer::createCommandBuffers() {
  commandBuffers.resize(LvsSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = lvsDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (VkResult result = vkAllocateCommandBuffers(lvsDevice.device(), &allocInfo, commandBuffers.data()); result != VK_SUCCESS) {
    throw std::runtime_error(cpc::Red + "Failed to allocate command buffer Error: " + std::to_string(result) + cpc::Reset);
  }
  std::cout << cpc::Green << "Successfully allocated command buffer" << cpc::Reset << std::endl;
}

void LvsRenderer::freeCommandBuffers() {
  vkFreeCommandBuffers(lvsDevice.device(), lvsDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
  commandBuffers.clear();
}

void LvsRenderer::initViewportConfig(LvsRenderer::ViewportConfig &viewportConfig) {
  nlohmann::json viewportSettings = jsf::getSettings(jsf::DISPLAY_SETTINGS)["viewport"];

  nlohmann::json startPoint = viewportSettings["End_Point"];
  nlohmann::json endPoint = viewportSettings["Start_Point"];

  bool defaultValues = viewportSettings["Same_as_Screen"];

  viewportConfig.height = defaultValues ? static_cast<float>(lvsSwapChain->getSwapChainExtent().height) : static_cast<float>(endPoint["Y"]);
  viewportConfig.width = defaultValues ? static_cast<float>(lvsSwapChain->getSwapChainExtent().width) : static_cast<float>(endPoint["X"]);
  viewportConfig.x = defaultValues ? 0.0f : static_cast<float>(startPoint["X"]);
  viewportConfig.y = defaultValues ? 0.0f : static_cast<float>(startPoint["Y"]);
  viewportConfig.minDepth = defaultValues ? 0.0f : static_cast<float>(viewportSettings["Min_Depth"]);
  viewportConfig.maxDepth = defaultValues ? 0.0f : static_cast<float>(viewportSettings["Max_Depth"]);
}

VkCommandBuffer LvsRenderer::beginFrame() {
  assert(!isFrameStarted && "Can't call beginFrame while already in progress");

  auto result = lvsSwapChain->acquireNextImage(&currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("Failed to acquire swap chain image Error: " + std::to_string(result));
  }

  isFrameStarted = true;
  
  auto commandBuffer = getCurrentCommandBuffer();
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
  if (result != VK_SUCCESS) {
    throw std::runtime_error(cpc::Red + "Failed to begin recording command buffer! Error: " + std::to_string(result) + cpc::Reset);
  }
  return commandBuffer;
}

void LvsRenderer::endFrame() {
  assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
  auto commandBuffer = getCurrentCommandBuffer();

  VkResult result = vkEndCommandBuffer(commandBuffer);
  if (result != VK_SUCCESS) {
    throw std::runtime_error(cpc::Red + "Failed to record command buffer! Error: " + std::to_string(result) + cpc::Reset);
  }

  result = lvsSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || result == lvsWindow.wasWindowResized()) {
    lvsWindow.resetWidnowResizedFlag();
    recreateSwapChain();
    isFrameStarted = false;
    return;
  }

  if (result != VK_SUCCESS) {
    throw std::runtime_error(cpc::Red + "Failed to submit command buffer Error: " + std::to_string(result) + cpc::Reset);
  }

  isFrameStarted = false;
  currentFrameIndex = (currentFrameIndex + 1) % LvsSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void LvsRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
  assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = lvsSwapChain->getRenderPass();
  renderPassInfo.framebuffer = lvsSwapChain->getFrameBuffer(currentImageIndex);
  
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = lvsSwapChain->getSwapChainExtent();

  std::vector<float> backgroundColor = static_cast<std::vector<float>>(jsf::getSettings(jsf::DISPLAY_SETTINGS)["Background_Color"]);

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]};
  clearValues[1].depthStencil = {1.0f, 0};

  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  LvsRenderer::ViewportConfig viewportConfig;
  LvsRenderer::initViewportConfig(viewportConfig);

  VkViewport viewport{};
  viewport.x = viewportConfig.x;
  viewport.y = viewportConfig.y;
  viewport.width = viewportConfig.width;
  viewport.height = viewportConfig.height;
  viewport.minDepth = viewportConfig.minDepth;
  viewport.maxDepth = viewportConfig.maxDepth;
  VkRect2D scissor{{0, 0}, lvsSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void LvsRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) { 
  assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
  assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

  vkCmdEndRenderPass(commandBuffer);
}

} // namespace lvs