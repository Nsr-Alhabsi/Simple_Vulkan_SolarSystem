#include "lvs_particle_system.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <algorithm>

namespace lvs {

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

  // Spawn callback
  if (m_EffectSoa.effect_on_particle_spawn[effect_idx] != nullptr)
    m_EffectSoa.effect_on_particle_spawn[effect_idx](m_EffectSoa.effect_callback_data[effect_idx]);
}

void LvsParticleSystem::updateParticlePosition(int idx) {
}

}
