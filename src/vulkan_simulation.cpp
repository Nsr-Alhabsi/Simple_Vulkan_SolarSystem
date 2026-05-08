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
  loadGameObjects();
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

void Simulation::loadGameObjects() {
  auto circle = LvsGameObject::createGameObject(ObjectType::Circle, lvsDevice);
  circle.transform2D.scale /= 2;
  circle.isGradient = true;

  gameObjects.push_back(std::move(circle));


  circle = LvsGameObject::createGameObject(ObjectType::Circle, lvsDevice);
  circle.transform2D.scale /= 5;
  circle.transform2D.translation.x = .3f;
  circle.color = {.25f, .25f, .25f};

  LvsGameAnimations::AnimationProperties circleAnimationProperties{};
  circleAnimationProperties.TARGET_ID = circle.getId();
  circleAnimationProperties.ANIMATION_NAME = "Circle_Orbit_Animation";
  circleAnimationProperties.TYPE = g_AnimationManager.ANIMATION_TYPE_ROTATION;

  circleAnimationProperties.ANIMATION_TYPE_ROTATION.PIVOT_POINT = {0.0f, 0.0f};
  circleAnimationProperties.ANIMATION_TYPE_ROTATION.RADIUS = glm::distance(
    circleAnimationProperties.ANIMATION_TYPE_ROTATION.PIVOT_POINT,
    circle.transform2D.translation
  );
  circleAnimationProperties.ANIMATION_TYPE_ROTATION.ANIMATION_ENDING_RADIAN = glm::two_pi<float>();

  circleAnimationProperties.ANIMATION_DURATION = 5.0f;
  circleAnimationProperties.ANIMATION_REPETITION = -1;

  g_AnimationManager.setAnimation(circleAnimationProperties);

  auto moon = LvsGameObject::createGameObject(ObjectType::Circle, lvsDevice);
  moon.transform2D.translation.x = .4f;
  moon.color = {0.5f, 0.5f, 0.5f};

  LvsGameAnimations::AnimationProperties moonAnimationProperties{};
  moonAnimationProperties.TARGET_ID = moon.getId();
  moonAnimationProperties.ANIMATION_NAME = "Moon_Orbit_Animation";
  moonAnimationProperties.TYPE = g_AnimationManager.ANIMATION_TYPE_ROTATION;

  moonAnimationProperties.ANIMATION_TYPE_ROTATION.RADIUS = 0.2f;
  moonAnimationProperties.ANIMATION_TYPE_ROTATION.ANIMATION_ENDING_RADIAN = glm::two_pi<float>();

  moonAnimationProperties.ANIMATION_DURATION = 2.f;
  moonAnimationProperties.ANIMATION_REPETITION = -1;

  g_AnimationManager.setAnimation(moonAnimationProperties);

  gameObjects.push_back(std::move(circle));
  gameObjects.push_back(std::move(moon));
}

}