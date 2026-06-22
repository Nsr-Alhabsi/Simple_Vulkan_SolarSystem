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
      lvsRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
      lvsRenderer.endSwapChainRenderPass(commandBuffer);
      lvsRenderer.endFrame();
    }
  }
}

void Simulation::loadObjects() {
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
  planet.transform2D.scale /= 3;
  planet.color = {.25f, .25f, .25f};
  planet.hasParent = true;
  planet.parentId = sunId;
  id_t planetId = planet.getId();

  LvsGameAnimations::AnimationProperties planetOrbit{};
  planetOrbit.TARGET_ID = planetId;
  planetOrbit.ANIMATION_NAME = "Planet_Orbit";
  planetOrbit.TYPE = g_AnimationManager.ANIMATION_TYPE_ROTATION;
  planetOrbit.rotation.pivot_point = {0.0f, 0.0f};
  planetOrbit.rotation.radius = 3.f;
  planetOrbit.rotation.ending_radian = glm::two_pi<float>();
  planetOrbit.duration = 6.f;
  planetOrbit.repetition = -1;
  g_AnimationManager.setAnimation(planetOrbit);

  // MOON
  auto moon = LvsGameObject::createGameObject(LvsGameObject::ObjectType::Circle, lvsDevice);
  moon.transform2D.scale /= 2;
  // moon.transform2D.translation.x = 1.5f; // Offset from Planet
  moon.color = {0.5f, 0.5f, 0.5f};
  moon.hasParent = true;
  moon.parentId = planetId;
  id_t moonId = moon.getId();

  LvsGameAnimations::AnimationProperties moonOrbit{};
  moonOrbit.TARGET_ID = moonId;
  moonOrbit.ANIMATION_NAME = "Moon_Orbit";
  moonOrbit.TYPE = g_AnimationManager.ANIMATION_TYPE_ROTATION;
  moonOrbit.rotation.radius = 2.3f;
  moonOrbit.rotation.ending_radian = glm::two_pi<float>();
  moonOrbit.duration = 2.f;
  moonOrbit.repetition = -1;
  g_AnimationManager.setAnimation(moonOrbit);

  gameObjects.emplace(planetId, std::move(planet));
  gameObjects.emplace(moonId, std::move(moon));
}

}