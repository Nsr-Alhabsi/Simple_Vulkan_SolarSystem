#include "vulkan_simulation.hpp"
#include "ADDONS/cp_color.hpp"
#include "ADDONS/json_setup.hpp"

#include "models/simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <chrono>

// std
#include <stdexcept>

namespace lvs { // lvs stands for large - vulkan - simulation

LvsGameAnimations g_AnimationManager;

Simulation::Simulation() {
  loadObjects();
}

Simulation::~Simulation() {}

void Simulation::updateFPS(std::chrono::high_resolution_clock::time_point &lastTime) {
  auto currentTime = std::chrono::high_resolution_clock::now();

  float dt = std::chrono::duration<float>(currentTime - lastTime).count();
  lastTime = currentTime;

  FrameInformation.deltaFrameTime = dt;

  float frameTimeMs = dt * 1000.0f;

  FrameInformation.totalFrames++;
  FrameInformation.totalTime += dt;

  if (frameTimeMs < FrameInformation.minFrameTimeMs) FrameInformation.minFrameTimeMs = frameTimeMs;
  if (frameTimeMs > FrameInformation.maxFrameTimeMs) FrameInformation.maxFrameTimeMs = frameTimeMs;

  FrameInformation.currentFPS = (dt > 0) ? (1.0f / dt) : 0.0f;

  FrameInformation.avgFrameTimeMs = (FrameInformation.totalTime * 1000.0f) / FrameInformation.totalFrames;
}

void Simulation::run() {
  SimpleRenderSystem simpleRenderSystem{lvsDevice, lvsRenderer.getSwapChainRenderPass()};
  auto lastTime = std::chrono::high_resolution_clock::now();

  while(!lvsWindow.shouldClose()) {
    glfwPollEvents();
    updateFPS(lastTime);

    if (auto commandBuffer = lvsRenderer.beginFrame()) {
      effectManager.updateEffects(FrameInformation.deltaFrameTime);
      lvsRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
      simpleRenderSystem.renderParticles(commandBuffer, effectManager);
      lvsRenderer.endSwapChainRenderPass(commandBuffer);
      lvsRenderer.endFrame();
    }
  }
}

LvsEffects::effectProperties Simulation::getEffectProperties() {
  return {};
}

void Simulation::loadObjects() {
  effectManager.init(10, 1000);
  morphManager.init(10);

  // SUN
  auto sun = LvsGameObject::createGameObject(LvsGameObject::ObjectType::Circle, lvsDevice);
  sun.transform2D.scale /= 4;
  sun.color = {1.f, .8f, 0.f};
  sun.color2 = {1.f, .2f, .0f};
  sun.gradDir = {0.f, 1.f};
  sun.isGradient = true;
  id_t sunId = sun.getId();
  gameObjects.emplace(sunId, std::move(sun));

  // PLANET
  auto planet = LvsGameObject::createGameObject(LvsGameObject::ObjectType::Circle, lvsDevice);
  planet.transform2D.scale /= 8;
  planet.color = {.25f, .25f, .25f};
  id_t planetId = planet.getId();

  LvsGameAnimations::AnimationProperties planetOrbit{};
  planetOrbit.TARGET_ID = planetId;
  planetOrbit.ANIMATION_NAME = "Planet_Orbit";
  planetOrbit.TYPE = g_AnimationManager.ANIMATION_TYPE_ROTATION;
  planetOrbit.rotation.pivot_point = {0.0f, 0.0f};
  planetOrbit.rotation.radius = 0.6f;
  planetOrbit.rotation.ending_radian = glm::two_pi<float>();
  planetOrbit.duration = 6.f;
  planetOrbit.repetition = -1;
  g_AnimationManager.setAnimation(planetOrbit);

  // MOON
  auto moon = LvsGameObject::createGameObject(LvsGameObject::ObjectType::Circle, lvsDevice);
  moon.transform2D.scale /= 5;
  moon.color = {0.5f, 0.5f, 0.5f};
  moon.hasParent = true;
  moon.parentId = planetId;
  id_t moonId = moon.getId();

  LvsGameAnimations::AnimationProperties moonOrbit{};
  moonOrbit.TARGET_ID = moonId;
  moonOrbit.ANIMATION_NAME = "Moon_Orbit";
  moonOrbit.TYPE = g_AnimationManager.ANIMATION_TYPE_ROTATION;
  moonOrbit.rotation.radius = 0.25f;
  moonOrbit.rotation.ending_radian = glm::two_pi<float>();
  moonOrbit.duration = 2.f;
  moonOrbit.repetition = -1;
  g_AnimationManager.setAnimation(moonOrbit);

  gameObjects.emplace(planetId, std::move(planet));
  gameObjects.emplace(moonId, std::move(moon));

  auto particleTemplate = LvsGameObject::createGameObject(LvsGameObject::ObjectType::Circle, lvsDevice);
  particleTemplate.transform2D.scale       = {0.015f, 0.015f};
  particleTemplate.transform2D.translation = {0.f, 0.f};
  particleTemplate.color = {1.f, 0.8f, 0.f};
  particleTemplate.visible = false;
  id_t particleTemplateId = particleTemplate.getId();
  gameObjects.emplace(particleTemplateId, std::move(particleTemplate));

  LvsEffects::effectProperties fx{};
  fx.particle               = &gameObjects.at(particleTemplateId);
  fx.emission_radius        = 0.27f;  
  fx.emit_from_edge         = true;   
  fx.emission_arc           = 360.f;
  fx.spawn_rate             = 25.f;
  fx.particle_duration      = 1.2f;
  fx.particle_velocity_start = 0.45f;
  fx.particle_velocity_end   = 0.f;
  fx.velocity_ease          = LvsEasingFunctions::EASE_OUT_QUAD;
  fx.particle_color_start   = {1.f, 0.8f, 0.f};
  fx.particle_color_end     = {1.f, 0.1f, 0.f};
  fx.particle_opacity_start  = 1.f;
  fx.particle_opacity_end    = 0.f;
  fx.fade_out_time          = 0.4f;
  fx.particle_scale_end     = {0.005f, 0.005f};
  fx.repetition             = -1;
  effectManager.initializeEffect(fx);

  // Demo morph: two same-vertex-count triangle shapes registered into the morph
  // pool. Proves out the full LvsMorph pipeline (vertex validation, SoA slot
  // allocation, property sync) end-to-end. Per-frame interpolation playback is
  // not yet implemented, so this shape does not animate visually.
  std::vector<LvsModel::Vertex> morphShapeA = {
    {{0.0f, -0.5f}, {1.f, 1.f, 1.f}, {0.5f, 0.0f}},
    {{-0.5f, 0.5f}, {1.f, 1.f, 1.f}, {0.0f, 1.0f}},
    {{0.5f, 0.5f},  {1.f, 1.f, 1.f}, {1.0f, 1.0f}},
  };
  std::vector<LvsModel::Vertex> morphShapeB = {
    {{0.0f, -0.2f}, {1.f, 1.f, 1.f}, {0.5f, 0.0f}},
    {{-0.2f, 0.2f}, {1.f, 1.f, 1.f}, {0.0f, 1.0f}},
    {{0.2f, 0.2f},  {1.f, 1.f, 1.f}, {1.0f, 1.0f}},
  };
  std::vector<std::vector<LvsModel::Vertex>> morphShapes = {morphShapeA, morphShapeB};

  LvsMorph::morphProperties morphProps{};
  morphProps.MORPH_NAME = "Demo_Triangle_Pulse";
  morphProps.duration = 1.5f;
  morphProps.repetition = -1;
  morphProps.sequence_mode = MORPH_SEQUENCE_PING_PONG;
  morphManager.morphObject(morphShapes, morphProps);
}

}
