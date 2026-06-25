#pragma once

#include "../core/lvs_pipeline.hpp"
#include "../ADDONS/json_setup.hpp"
#include "lvs_model.hpp"
#include "lvs_game_object.hpp"
#include "properties/lvs_game_animations.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <vector>

namespace lvs {

class LvsEffectManager;

class SimpleRenderSystem {
public:
  SimpleRenderSystem(LvsDevice& device, VkRenderPass renderPass);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem& operator=(const SimpleRenderSystem &) = delete;

  void renderGameObjects(VkCommandBuffer commandBuffer, std::unordered_map<LvsGameObject::id_t, LvsGameObject>& gameObjects);
  void renderParticles(VkCommandBuffer commandBuffer, LvsEffectManager& manager);
private:
  void createPipelineLayout();
  void createPipeline(VkRenderPass renderPass);

  LvsDevice &lvsDevice;
  LvsGameAnimations lvsGameAnimations;

  std::unique_ptr<LvsPipeline> lvsPipeline;
  VkPipelineLayout pipelineLayout;
};

}