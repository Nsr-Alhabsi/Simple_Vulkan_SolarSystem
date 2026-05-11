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
  glm::mat4 transform{1.f};

  alignas(16) glm::vec3 color1;
  alignas(16) glm::vec3 color2;
  
  alignas(8) glm::vec2 gradDir;
  alignas(4) int useGradient;
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

void SimpleRenderSystem::renderGameObjects(
  VkCommandBuffer commandBuffer,
  std::unordered_map<unsigned int, LvsGameObject> &gameObjects) 
  {  
  lvsPipeline->bind(commandBuffer);

  g_AnimationManager.updateAnimations(gameObjects);
  for (auto& kv: gameObjects) {
    auto& obj = kv.second;
    if (obj.model == nullptr) continue;

    simplePushConstantData push{};
    push.color1 = obj.color;
    push.color2 = obj.color2;
    push.gradDir = obj.gradDir;

    push.transform = glm::mat4{obj.getGlobalMatrix(gameObjects)};
    
    push.useGradient = obj.isGradient ? 1 : 0;

    vkCmdPushConstants(commandBuffer, pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0, sizeof(simplePushConstantData), &push);
    obj.model->bind(commandBuffer);
    obj.model->draw(commandBuffer);
  }
}

}