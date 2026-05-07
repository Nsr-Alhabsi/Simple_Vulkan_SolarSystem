#include "simple_render_system.hpp"
#include "../ADDONS/cp_color.hpp"
#include "../ADDONS/json_setup.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

// std
#include <stdexcept>

struct simplePushConstantData {
  glm::mat2 transform{1.f};
  alignas(8) glm::vec2 offset;
  alignas(16) glm::vec3 color;
  bool useGradient;
};

namespace lvs { // lvs stands for large - vulkan - simulation

SimpleRenderSystem::SimpleRenderSystem(LvsDevice& device, VkRenderPass renderPass) : lvsDevice{device} {
  createPipelineLayout();
  createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(lvsDevice.device(), pipelineLayout, nullptr); }

void SimpleRenderSystem::createPipelineLayout() {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(simplePushConstantData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  
  if (VkResult result = vkCreatePipelineLayout(lvsDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout); result != VK_SUCCESS) {
    throw std::runtime_error(cpc::Red + "Failed to create pipeline layout Error: " + std::to_string(result) + cpc::Reset);
  }
  std::cout << cpc::Green << "Successfully created pipeline layout" << cpc::Reset << std::endl;
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  LvsPipeline::PipelineConfigInfo pipelineConfig{};
  LvsPipeline::defaultPipelineConfigInfo(pipelineConfig);

  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  lvsPipeline = std::make_unique<LvsPipeline>(
    lvsDevice,
    "shaders/simple_shader.vert.spv",
    "shaders/simple_shader.frag.spv",
    pipelineConfig);
}

void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LvsGameObject> &gameObjects) {
  lvsPipeline->bind(commandBuffer);

  g_AnimationManager.updateAnimations(gameObjects);
  for (auto& obj: gameObjects) {
    simplePushConstantData push{};
    push.offset = obj.transform2D.translation;
    push.color = obj.color;
    push.transform = obj.transform2D.mat2();
    push.useGradient = obj.isGradient;

    vkCmdPushConstants(commandBuffer, pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0, sizeof(simplePushConstantData), &push);
    obj.model->bind(commandBuffer);
    obj.model->draw(commandBuffer);
  }
}

}