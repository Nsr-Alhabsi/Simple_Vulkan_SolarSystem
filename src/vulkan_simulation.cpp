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

      morphCalculator.calculateMorph(emberMorphShapes, emberMorphProps, FrameInformation.deltaFrameTime);
      if (emberMorphProps.TARGET_OBJECT) {
        emberMorphProps.TARGET_OBJECT->setVertices(emberMorphProps.current_vertices);
      }

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

  // Ember particle shape: a small circle-fan (same construction as the built-in Circle model —
  // see LvsGameObject::createObjectVertices case 0 — but as its own ObjectType::Custom mesh, not
  // the shared Circle singleton every other Circle object uses). It has to be Custom: setVertices
  // refuses to touch built-in Circle/Triangle/Square objects specifically because they share one
  // static model across the whole scene, and mutating it would deform the sun/planet/moon too.
  //
  // UV is derived from position the same way createObjectVertices does it (uv = pos*0.5+0.5),
  // which is what makes the Circle model render as a full, unclipped disk against
  // simple_shader.frag's "discard if UV distance from center > 0.5" rule. Keeping that same
  // relationship for the wobbly variant (radius never exceeds 1.0) means the wobble never gets
  // clipped either — it reads as a soft, organic pulsing blob rather than an artifact.
  auto makeEmberShape = [](int segments, auto radiusFn) {
    std::vector<LvsModel::Vertex> verts;
    verts.reserve(static_cast<size_t>(segments) * 3);
    const float angleStep = glm::two_pi<float>() / static_cast<float>(segments);
    for (int i = 0; i < segments; i++) {
      float a1 = static_cast<float>(i) * angleStep;
      float a2 = static_cast<float>(i + 1) * angleStep;
      float r1 = radiusFn(a1);
      float r2 = radiusFn(a2);
      glm::vec2 p0 = {0.0f, 0.0f};
      glm::vec2 p1 = {r1 * glm::cos(a1), r1 * glm::sin(a1)};
      glm::vec2 p2 = {r2 * glm::cos(a2), r2 * glm::sin(a2)};
      auto getUV = [](glm::vec2 p) { return p * 0.5f + 0.5f; };
      verts.push_back({p0, {1.f, 1.f, 1.f}, getUV(p0)});
      verts.push_back({p2, {1.f, 1.f, 1.f}, getUV(p2)});
      verts.push_back({p1, {1.f, 1.f, 1.f}, getUV(p1)});
    }
    return verts;
  };

  // Shape A: a plain circle (radius 1.0 all the way around, matching the built-in Circle model).
  std::vector<LvsModel::Vertex> emberShapeA = makeEmberShape(24, [](float) { return 1.0f; });
  // Shape B: the same circle with a 4-lobed inward wobble (radius dips to 0.68 at its narrowest)
  // — never exceeds 1.0, so it stays inside the built-in model's original silhouette and never
  // gets clipped. The dip has to be this large (not the originally-tried 0.88) because these
  // particles render at only ~10-15px on screen: a 12%-radius wobble there is sub-pixel and
  // invisible, even though it's clearly visible on a larger test shape at the same relative
  // amplitude — the effect has to be tuned against actual on-screen particle size, not just the
  // shape's own local unit circle.
  std::vector<LvsModel::Vertex> emberShapeB = makeEmberShape(24, [](float a) {
    return 1.0f - 0.32f * (0.5f + 0.5f * glm::sin(4.0f * a));
  });

  auto particleTemplate = LvsGameObject::createGameObject(LvsGameObject::ObjectType::Custom, lvsDevice, &emberShapeA);
  particleTemplate.transform2D.scale       = {0.022f, 0.022f}; // slightly larger than before so the wobble has more pixels to read in
  particleTemplate.transform2D.translation = {0.f, 0.f};
  particleTemplate.transform2D.rotation    = 0.0f; // Transform2DComponent::rotation has no default initializer
  particleTemplate.color = {1.f, 0.8f, 0.f};
  particleTemplate.visible = false;
  id_t particleTemplateId = particleTemplate.getId();
  gameObjects.emplace(particleTemplateId, std::move(particleTemplate));

  emberMorphShapes = {emberShapeA, emberShapeB};
  emberMorphProps = LvsMorph::morphProperties{};
  emberMorphProps.MORPH_NAME = "Ember_Shimmer";
  emberMorphProps.TARGET_OBJECT = &gameObjects.at(particleTemplateId);
  emberMorphProps.duration = 0.22f; // fast enough to read as a flicker, not a slow breathing pulse
  emberMorphProps.repetition = -1;
  emberMorphProps.sequence_mode = MORPH_SEQUENCE_FORWARD;
  emberMorphProps.reverse_on_finish = true; // bounces circle <-> wobble <-> circle continuously
  emberMorphProps.morph_vertex_uvs = true;  // keep uv = pos*0.5+0.5 exact at every mid-morph frame
  // position_ease and uv_ease must stay equal (both LINEAR by default here) so posT == uvT at
  // every tick — that's what keeps uv = pos*0.5+0.5 exact through the whole interpolation and
  // avoids clipping artifacts mid-wobble (see calculateMorph's posT/uvT split).
  morphManager.morphObject(emberMorphShapes, emberMorphProps);

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
}

}
