#pragma once

#include "../core/lvs_device.hpp"
#include "../core/lvs_swap_chain.hpp"
#include "../core/window_setup.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <vector>
#include <cassert>

namespace lvs {

class LvsRenderer {
struct ViewportConfig {
  float x, y, width, height, minDepth, maxDepth;
};

public:
  LvsRenderer(LvsWindow &window, LvsDevice &device);
  ~LvsRenderer();

  LvsRenderer(const LvsRenderer &) = delete;
  LvsRenderer& operator=(const LvsRenderer &) = delete;

  VkRenderPass getSwapChainRenderPass() const {return lvsSwapChain->getRenderPass(); }

  bool isFrameInProgress() const { return isFrameStarted; }
  VkCommandBuffer getCurrentCommandBuffer() const { 
    assert(isFrameStarted && "Cannot get command buffer when frame not in progess");
    return commandBuffers[currentFrameIndex];
  }

  int getFrameIndex() const {
    assert(isFrameStarted && "Cannot get frame index when frame not in progress");
    return currentFrameIndex;
  }

  VkCommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();
  void initViewportConfig(LvsRenderer::ViewportConfig &viewportConfig);

  LvsWindow& lvsWindow;
  LvsDevice& lvsDevice;
  std::unique_ptr<LvsSwapChain> lvsSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;

  uint32_t currentImageIndex{0};
  int currentFrameIndex{0};
  bool isFrameStarted{false};
};

}