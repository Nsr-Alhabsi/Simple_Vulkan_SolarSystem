#include "lvs_particle_system.hpp"

// ============================================================
//  INCLUDES
// ============================================================

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <algorithm>

namespace lvs {

// ============================================================
//  CONSTANTS / STATICS
// ============================================================

// Dispatches to the correct easing function. Falls back to the custom
// function pointer when ease == CUSTOM_CURVE_STEER and the pointer is set.
static float applyEase(LvsEasingFunctions::EaseType ease, float(*customFn)(float), float t) {
  using E = LvsEasingFunctions;
  if (ease == E::CUSTOM_CURVE_STEER && customFn) return customFn(t);
  static float(*const TABLE[])(float) = {
    E::LINEAR_FUNCTION,
    E::EASE_IN_SINE_FUNCTION,     E::EASE_OUT_SINE_FUNCTION,     E::EASE_IN_OUT_SINE_FUNCTION,
    E::EASE_IN_QUAD_FUNCTION,     E::EASE_OUT_QUAD_FUNCTION,     E::EASE_IN_OUT_QUAD_FUNCTION,
    E::EASE_IN_CUBIC_FUNCTION,    E::EASE_OUT_CUBIC_FUNCTION,    E::EASE_IN_OUT_CUBIC_FUNCTION,
    E::EASE_IN_QUART_FUNCTION,    E::EASE_OUT_QUART_FUNCTION,    E::EASE_IN_OUT_QUART_FUNCTION,
    E::EASE_IN_QUINT_FUNCTION,    E::EASE_OUT_QUINT_FUNCTION,    E::EASE_IN_OUT_QUINT_FUNCTION,
    E::EASE_IN_EXPO_FUNCTION,     E::EASE_OUT_EXPO_FUNCTION,     E::EASE_IN_OUT_EXPO_FUNCTION,
    E::EASE_IN_CIRC_FUNCTION,     E::EASE_OUT_CIRC_FUNCTION,     E::EASE_IN_OUT_CIRC_FUNCTION,
    E::EASE_IN_BACK_FUNCTION,     E::EASE_OUT_BACK_FUNCTION,     E::EASE_IN_OUT_BACK_FUNCTION,
    E::EASE_IN_ELASTIC_FUNCTION,  E::EASE_OUT_ELASTIC_FUNCTION,  E::EASE_IN_OUT_ELASTIC_FUNCTION,
    E::EASE_IN_BOUNCE_FUNCTION,   E::EASE_OUT_BOUNCE_FUNCTION,   E::EASE_IN_OUT_BOUNCE_FUNCTION,
  };
  return TABLE[ease](t);
}

// ============================================================
//  PUBLIC METHODS
// ============================================================

void LvsParticleSystem::initalizeParticle(int effect_idx, int local_slot) {
  int abs_idx = m_EffectSoa.effect_particle_pool_effect[effect_idx] + local_slot;

  m_ParticleSoa.p_alive[abs_idx]      = true;
  m_ParticleSoa.p_effect_idx[abs_idx] = effect_idx;
  m_ParticleSoa.p_local_slot[abs_idx] = local_slot;
  m_ParticleSoa.p_age[abs_idx]        = 0.f;

  // Lifetime
  float baseDuration = m_EffectSoa.effect_particle_duration[effect_idx];
  if (baseDuration == -1.f) {
    m_ParticleSoa.p_lifetime[abs_idx] = -1.f;
  } else {
    float var = m_EffectSoa.effect_particle_duration_variance[effect_idx];
    std::uniform_real_distribution<float> durDist(-var, var);
    m_ParticleSoa.p_lifetime[abs_idx] = std::max(0.f, baseDuration + durDist(m_Rng));
  }

  // Delay
  float delayVar = m_EffectSoa.effect_particle_delay_variance[effect_idx];
  std::uniform_real_distribution<float> delayDist(0.f, std::max(0.f, delayVar));
  m_ParticleSoa.p_delay_remaining[abs_idx] = delayDist(m_Rng);

  // Spawn position
  glm::vec2 emitterPos = m_EffectSoa.effect_particle[effect_idx]->transform2D.translation;

  float arcHalf   = m_EffectSoa.effect_emission_arc[effect_idx] * 0.5f;
  float arcOffset = m_EffectSoa.effect_emission_arc_offset[effect_idx];
  std::uniform_real_distribution<float> arcDist(-arcHalf, arcHalf);
  float spawnAngleDeg = arcOffset + arcDist(m_Rng);
  float spawnAngleRad = glm::radians(spawnAngleDeg);

  float radius = m_EffectSoa.effect_emission_radius[effect_idx];
  float spawnRadius;
  if (m_EffectSoa.effect_emit_from_edge[effect_idx] || radius == 0.f) {
    spawnRadius = radius;
  } else {
    std::uniform_real_distribution<float> radDist(0.f, radius);
    spawnRadius = radDist(m_Rng);
  }

  glm::vec2 spawnOffset = glm::vec2(std::cos(spawnAngleRad), std::sin(spawnAngleRad)) * spawnRadius;

  glm::vec2 posVar = m_EffectSoa.effect_starting_position_variance[effect_idx];
  std::uniform_real_distribution<float> posVarX(-posVar.x, posVar.x);
  std::uniform_real_distribution<float> posVarY(-posVar.y, posVar.y);

  glm::vec2 spawnPos = emitterPos + spawnOffset + glm::vec2(posVarX(m_Rng), posVarY(m_Rng));
  m_ParticleSoa.p_position[abs_idx]       = spawnPos;
  m_ParticleSoa.p_spawn_position[abs_idx] = spawnPos;

  // Direction
  float dirVar = m_EffectSoa.effect_direction_variance[effect_idx];
  std::uniform_real_distribution<float> dirDist(-dirVar, dirVar);
  float baseDir = (m_EffectSoa.effect_emission_arc[effect_idx] >= 360.f)
                  ? glm::degrees(std::atan2(spawnOffset.y, spawnOffset.x))
                  : spawnAngleDeg;
  m_ParticleSoa.p_direction[abs_idx] = baseDir + dirDist(m_Rng);

  // Velocity
  float velVar = m_EffectSoa.effect_velocity_variance[effect_idx];
  std::uniform_real_distribution<float> velDist(-velVar, velVar);
  m_ParticleSoa.p_velocity[abs_idx] = std::max(0.f, m_EffectSoa.effect_particle_velocity_start[effect_idx] + velDist(m_Rng));

  // Angular velocity
  float angVar = m_EffectSoa.effect_angular_velocity_variance[effect_idx];
  std::uniform_real_distribution<float> angDist(-angVar, angVar);
  m_ParticleSoa.p_angular_vel[abs_idx] = m_EffectSoa.effect_particle_angular_velocity[effect_idx] + angDist(m_Rng);

  // Scale
  glm::vec2 scaleVar = m_EffectSoa.effect_scale_variance[effect_idx];
  std::uniform_real_distribution<float> scaleVarX(-scaleVar.x, scaleVar.x);
  std::uniform_real_distribution<float> scaleVarY(-scaleVar.y, scaleVar.y);
  m_ParticleSoa.p_scale[abs_idx] = m_EffectSoa.effect_particle[effect_idx]->transform2D.scale
                                  + glm::vec2(scaleVarX(m_Rng), scaleVarY(m_Rng));

  // Color
  glm::vec3 colorVar = m_EffectSoa.effect_color_start_variance[effect_idx];
  std::uniform_real_distribution<float> colorVarR(-colorVar.r, colorVar.r);
  std::uniform_real_distribution<float> colorVarG(-colorVar.g, colorVar.g);
  std::uniform_real_distribution<float> colorVarB(-colorVar.b, colorVar.b);
  m_ParticleSoa.p_color[abs_idx] = glm::clamp(
    m_EffectSoa.effect_particle_color_start[effect_idx] + glm::vec3(colorVarR(m_Rng), colorVarG(m_Rng), colorVarB(m_Rng)),
    0.f, 1.f
  );

  // Opacity
  float opVar = m_EffectSoa.effect_opacity_variance[effect_idx];
  std::uniform_real_distribution<float> opDist(-opVar, opVar);
  m_ParticleSoa.p_opacity[abs_idx] = std::clamp(
    m_EffectSoa.effect_particle_opacity_start[effect_idx] + opDist(m_Rng),
    0.f, 1.f
  );

  // Snapshot spawned values so update functions have stable interpolation baselines
  m_ParticleSoa.p_velocity_start[abs_idx]    = m_ParticleSoa.p_velocity[abs_idx];
  m_ParticleSoa.p_angular_vel_start[abs_idx] = m_ParticleSoa.p_angular_vel[abs_idx];
  m_ParticleSoa.p_scale_start[abs_idx]       = m_ParticleSoa.p_scale[abs_idx];
  m_ParticleSoa.p_color_start[abs_idx]       = m_ParticleSoa.p_color[abs_idx];
  m_ParticleSoa.p_opacity_start[abs_idx]     = m_ParticleSoa.p_opacity[abs_idx];

  // Spawn callback
  if (m_EffectSoa.effect_on_particle_spawn[effect_idx] != nullptr)
    m_EffectSoa.effect_on_particle_spawn[effect_idx](m_EffectSoa.effect_callback_data[effect_idx]);
}

void LvsParticleSystem::updateParticlePosition(int abs_idx, float dt) {
  m_ParticleSoa.p_age[abs_idx] += dt;

  int eidx     = m_ParticleSoa.p_effect_idx[abs_idx];
  float age     = m_ParticleSoa.p_age[abs_idx];
  float lifetime = m_ParticleSoa.p_lifetime[abs_idx];

  float t = 0.f;
  if (lifetime > 0.f) t = std::clamp(age / lifetime, 0.f, 1.f);

  // Color interpolation
  float colorT = (lifetime != -1.f)
    ? applyEase(m_EffectSoa.effect_color_ease[eidx], m_EffectSoa.effect_color_custom_ease_function[eidx], t)
    : 0.f;
  m_ParticleSoa.p_color[abs_idx] = glm::mix(m_ParticleSoa.p_color_start[abs_idx],
                                              m_EffectSoa.effect_particle_color_end[eidx],
                                              colorT);

  // Opacity interpolation with fade-in / fade-out
  float opacity = glm::mix(m_ParticleSoa.p_opacity_start[abs_idx],
                            m_EffectSoa.effect_particle_opacity_end[eidx],
                            colorT);
  if (lifetime > 0.f) {
    float fadeIn  = m_EffectSoa.effect_fade_in_time[eidx];
    float fadeOut = m_EffectSoa.effect_fade_out_time[eidx];
    if (fadeIn  > 0.f && age < fadeIn)              opacity *= age / fadeIn;
    else if (fadeOut > 0.f && age > lifetime - fadeOut) opacity *= std::max(0.f, (lifetime - age) / fadeOut);
  }
  m_ParticleSoa.p_opacity[abs_idx] = std::clamp(opacity, 0.f, 1.f);

  updateParticleTranslation(abs_idx, dt);
  updateParticleRotation(abs_idx, dt);
  updateParticleScale(abs_idx, dt);
}

// ============================================================
//  PRIVATE METHODS / HELPERS
// ============================================================

void LvsParticleSystem::updateParticleTranslation(int abs_idx, float dt) {
  int eidx = m_ParticleSoa.p_effect_idx[abs_idx];

  float t = 0.f;
  float lifetime = m_ParticleSoa.p_lifetime[abs_idx];
  if (lifetime > 0.f)
    t = std::clamp(m_ParticleSoa.p_age[abs_idx] / lifetime, 0.f, 1.f);

  float easedT = (lifetime != -1.f)
    ? applyEase(m_EffectSoa.effect_velocity_ease[eidx], m_EffectSoa.effect_velocity_custom_ease_function[eidx], t)
    : 0.f;

  float speed = glm::mix(m_ParticleSoa.p_velocity_start[abs_idx],
                          m_EffectSoa.effect_particle_velocity_end[eidx],
                          easedT);
  speed *= std::max(0.f, 1.f - m_EffectSoa.effect_drag[eidx] * dt);

  float dirRad = glm::radians(m_ParticleSoa.p_direction[abs_idx]);
  m_ParticleSoa.p_position[abs_idx] += glm::vec2(std::cos(dirRad), std::sin(dirRad)) * speed * dt;
  m_ParticleSoa.p_position[abs_idx] += m_EffectSoa.effect_particle__acceleration[eidx] * dt;
  m_ParticleSoa.p_position[abs_idx].y -= m_EffectSoa.effect_gravity_strength[eidx] * dt;
}

void LvsParticleSystem::updateParticleRotation(int abs_idx, float dt) {
  int eidx = m_ParticleSoa.p_effect_idx[abs_idx];

  float t = 0.f;
  float lifetime = m_ParticleSoa.p_lifetime[abs_idx];
  if (lifetime > 0.f)
    t = std::clamp(m_ParticleSoa.p_age[abs_idx] / lifetime, 0.f, 1.f);

  float easedT = (lifetime != -1.f)
    ? applyEase(m_EffectSoa.effect_velocity_ease[eidx], m_EffectSoa.effect_velocity_custom_ease_function[eidx], t)
    : 0.f;

  float currentAngVel = glm::mix(m_ParticleSoa.p_angular_vel_start[abs_idx],
                                   m_EffectSoa.effect_particle_angular_velocity_end[eidx],
                                   easedT);
  m_ParticleSoa.p_direction[abs_idx] += currentAngVel * dt;
}

void LvsParticleSystem::updateParticleScale(int abs_idx, float dt) {
  int eidx = m_ParticleSoa.p_effect_idx[abs_idx];

  float t = 0.f;
  float lifetime = m_ParticleSoa.p_lifetime[abs_idx];
  if (lifetime > 0.f)
    t = std::clamp(m_ParticleSoa.p_age[abs_idx] / lifetime, 0.f, 1.f);

  float easedT = (lifetime != -1.f)
    ? applyEase(m_EffectSoa.effect_scale_ease[eidx], m_EffectSoa.effect_scale_custom_ease_function[eidx], t)
    : 0.f;

  m_ParticleSoa.p_scale[abs_idx] = glm::mix(m_ParticleSoa.p_scale_start[abs_idx],
                                              m_EffectSoa.effect_particle_scale_end[eidx],
                                              easedT);
  (void)dt;
}

} // namespace lvs
