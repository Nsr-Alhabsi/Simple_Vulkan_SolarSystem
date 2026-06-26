#include "simple_render_system.hpp"
#include "../visuals/lvs_effect_manager.hpp"
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
  float opacity{1.f};
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
    if (!obj.visible || obj.model == nullptr) continue;

    simplePushConstantData push{};
    push.color1 = obj.color;
    push.color2 = obj.color2;
    push.gradDir = obj.gradDir;

    glm::mat3 m = obj.getGlobalMatrix(gameObjects);
    glm::mat4 t{1.0f};
    t[0][0] = m[0][0]; t[0][1] = m[0][1];
    t[1][0] = m[1][0]; t[1][1] = m[1][1];
    t[3][0] = m[2][0]; t[3][1] = m[2][1];  
    push.transform = t;
    
    push.useGradient = obj.isGradient ? 1 : 0;

    vkCmdPushConstants(commandBuffer, pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0, sizeof(simplePushConstantData), &push);
    obj.model->bind(commandBuffer);
    obj.model->draw(commandBuffer);
  }
}

void SimpleRenderSystem::renderParticles(VkCommandBuffer commandBuffer, LvsEffectManager& manager) {
  const auto& effectSoa   = manager.getEffectSoA();
  const auto& particleSoa = manager.getParticleSoA();

  lvsPipeline->bind(commandBuffer);

  for (int effect_idx : effectSoa.active_indices) {
    LvsGameObject* tmpl = effectSoa.effect_particle[effect_idx];
    if (!tmpl || !tmpl->model) continue;

    int pool_base = effectSoa.effect_particle_pool_effect[effect_idx];
    int pool_cap  = effectSoa.effect_max_simultaneous_particles[effect_idx];

    for (int local = 0; local < pool_cap; ++local) {
      int abs = pool_base + local;
      if (!particleSoa.p_alive[abs])               continue;
      if (particleSoa.p_delay_remaining[abs] > 0.f) continue;

      glm::vec2 pos   = particleSoa.p_position[abs];
      glm::vec2 scale = particleSoa.p_scale[abs];
      glm::vec3 color = particleSoa.p_color[abs];

      simplePushConstantData push{};
      push.color1      = color;
      push.color2      = tmpl->color2;
      push.gradDir     = tmpl->gradDir;
      push.useGradient = tmpl->isGradient ? 1 : 0;
      push.opacity     = particleSoa.p_opacity[abs];

      glm::mat4 t{1.f};
      t[0][0] = scale.x;
      t[1][1] = scale.y;
      t[3][0] = pos.x;
      t[3][1] = pos.y;
      push.transform = t;

      vkCmdPushConstants(commandBuffer, pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0, sizeof(simplePushConstantData), &push);
      tmpl->model->bind(commandBuffer);
      tmpl->model->draw(commandBuffer);
    }
  }
}

}