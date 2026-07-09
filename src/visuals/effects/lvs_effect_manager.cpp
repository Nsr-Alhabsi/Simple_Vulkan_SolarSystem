#include "lvs_effect_manager.hpp"

// ============================================================
//  INCLUDES
// ============================================================

#include "core/lvs_effects.hpp"

#include "../../ADDONS/cp_color.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace lvs {

// ============================================================
//  PUBLIC METHODS
// ============================================================

void LvsEffectManager::init(uint32_t count, uint32_t max_particles_total) {
  m_MaxEffects = count;
  m_MaxParticles = max_particles_total;
  m_ParticlePoolCursor = 0;
  m_ParticleFreeSlots.resize(count);

  // --- General ---
  soa.effect_EASE = std::make_unique<LvsEasingFunctions::EaseType[]>(count);

  // --- Particle reference ---
  soa.effect_particle = std::make_unique<LvsGameObject*[]>(count);

  // --- Spawn / Emission ---
  soa.effect_particle_ending_position = std::make_unique<glm::vec2[]>(count);
  soa.effect_emission_radius          = std::make_unique<float[]>(count);
  soa.effect_emission_arc             = std::make_unique<float[]>(count);
  soa.effect_emission_arc_offset      = std::make_unique<float[]>(count);
  soa.effect_emit_from_edge           = std::make_unique<bool[]>(count);
  soa.effect_spawn_rate               = std::make_unique<float[]>(count);
  soa.effect_repetition               = std::make_unique<int[]>(count);
  soa.effect_reverse_on_finish        = std::make_unique<bool[]>(count);
  soa.effect_max_presistent_particles = std::make_unique<int[]>(count);
  soa.effect_active                   = std::make_unique<bool[]>(count);
  soa.effect_spawn_accumulator        = std::make_unique<float[]>(count);

  // --- Motion / Physics ---
  soa.effect_particle_velocity_start       = std::make_unique<float[]>(count);
  soa.effect_particle_velocity_end         = std::make_unique<float[]>(count);
  soa.effect_particle__acceleration        = std::make_unique<glm::vec2[]>(count);
  soa.effect_gravity_strength              = std::make_unique<float[]>(count);
  soa.effect_particle_direction_end        = std::make_unique<float[]>(count);
  soa.effect_particle_angular_velocity     = std::make_unique<float[]>(count);
  soa.effect_particle_angular_velocity_end = std::make_unique<float[]>(count);
  soa.effect_drag                          = std::make_unique<float[]>(count);
  soa.effect_velocity_ease                 = std::make_unique<LvsEasingFunctions::EaseType[]>(count);
  soa.effect_velocity_custom_ease_function = std::make_unique<float(*[])(float)>(count);

  // --- Lifetime ---
  soa.effect_particle_duration          = std::make_unique<float[]>(count);
  soa.effect_particle_duration_variance = std::make_unique<float[]>(count);
  soa.effect_particle_delay_variance    = std::make_unique<float[]>(count);
  soa.effect_fade_in_time               = std::make_unique<float[]>(count);
  soa.effect_fade_out_time              = std::make_unique<float[]>(count);
  soa.effect_fade_ease                  = std::make_unique<LvsEasingFunctions::EaseType[]>(count);
  soa.effect_destroy_on_finish          = std::make_unique<bool[]>(count);

  // --- Appearance ---
  soa.effect_particle_color_start       = std::make_unique<glm::vec3[]>(count);
  soa.effect_particle_color_end         = std::make_unique<glm::vec3[]>(count);
  soa.effect_particle_color2_start      = std::make_unique<glm::vec3[]>(count);
  soa.effect_particle_color2_end        = std::make_unique<glm::vec3[]>(count);
  soa.effect_color_ease                 = std::make_unique<LvsEasingFunctions::EaseType[]>(count);
  soa.effect_use_gradient               = std::make_unique<bool[]>(count);
  soa.effect_gradient_direction_start   = std::make_unique<glm::vec2[]>(count);
  soa.effect_gradient_direction_end     = std::make_unique<glm::vec2[]>(count);
  soa.effect_particle_opacity_start     = std::make_unique<float[]>(count);
  soa.effect_particle_opacity_end       = std::make_unique<float[]>(count);
  soa.effect_particle_scale_end         = std::make_unique<glm::vec2[]>(count);
  soa.effect_scale_ease                 = std::make_unique<LvsEasingFunctions::EaseType[]>(count);
  soa.effect_color_custom_ease_function = std::make_unique<float(*[])(float)>(count);
  soa.effect_scale_custom_ease_function = std::make_unique<float(*[])(float)>(count);

  // --- Looping / Repetition ---
  soa.effect_loop_delay         = std::make_unique<float[]>(count);
  soa.effect_burst_mode         = std::make_unique<bool[]>(count);
  soa.effect_burst_count        = std::make_unique<int[]>(count);
  soa.effect_burst_interval     = std::make_unique<float[]>(count);
  soa.effect_burst_timer        = std::make_unique<float[]>(count);
  soa.effect_current_repetition = std::make_unique<int[]>(count);
  soa.effect_loop_delay_remaining = std::make_unique<float[]>(count);

  // --- Randomness / Variance ---
  soa.effect_random_seed                = std::make_unique<uint32_t[]>(count);
  soa.effect_velocity_variance          = std::make_unique<float[]>(count);
  soa.effect_direction_variance         = std::make_unique<float[]>(count);
  soa.effect_angular_velocity_variance  = std::make_unique<float[]>(count);
  soa.effect_scale_variance             = std::make_unique<glm::vec2[]>(count);
  soa.effect_color_start_variance       = std::make_unique<glm::vec3[]>(count);
  soa.effect_color_end_variance         = std::make_unique<glm::vec3[]>(count);
  soa.effect_opacity_variance           = std::make_unique<float[]>(count);
  soa.effect_starting_position_variance = std::make_unique<glm::vec2[]>(count);

  // --- Callbacks ---
  soa.effect_callback_data            = std::make_unique<void*[]>(count);
  soa.effect_on_effect_finish         = std::make_unique<void(*[])(void*)>(count);
  soa.effect_on_particle_spawn        = std::make_unique<void(*[])(void*)>(count);
  soa.effect_on_particle_death        = std::make_unique<void(*[])(void*)>(count);
  soa.effect_EFFECT_CUSTOM_EASE_FUNCTION = std::make_unique<float(*[])(float)>(count);

  soa.effect_max_simultaneous_particles = std::make_unique<int[]>(count);
  soa.effect_particle_pool_effect       = std::make_unique<int[]>(count);

  // --- Particle pool ---
  particleSoa.p_alive           = std::make_unique<bool[]>(max_particles_total);
  particleSoa.p_effect_idx      = std::make_unique<int[]>(max_particles_total);
  particleSoa.p_local_slot      = std::make_unique<int[]>(max_particles_total);
  particleSoa.p_position        = std::make_unique<glm::vec2[]>(max_particles_total);
  particleSoa.p_spawn_position  = std::make_unique<glm::vec2[]>(max_particles_total);
  particleSoa.p_velocity        = std::make_unique<float[]>(max_particles_total);
  particleSoa.p_direction       = std::make_unique<float[]>(max_particles_total);
  particleSoa.p_angular_vel     = std::make_unique<float[]>(max_particles_total);
  particleSoa.p_scale           = std::make_unique<glm::vec2[]>(max_particles_total);
  particleSoa.p_color           = std::make_unique<glm::vec3[]>(max_particles_total);
  particleSoa.p_opacity         = std::make_unique<float[]>(max_particles_total);
  particleSoa.p_age             = std::make_unique<float[]>(max_particles_total);
  particleSoa.p_lifetime        = std::make_unique<float[]>(max_particles_total);
  particleSoa.p_delay_remaining     = std::make_unique<float[]>(max_particles_total);
  particleSoa.p_velocity_start      = std::make_unique<float[]>(max_particles_total);
  particleSoa.p_angular_vel_start   = std::make_unique<float[]>(max_particles_total);
  particleSoa.p_scale_start         = std::make_unique<glm::vec2[]>(max_particles_total);
  particleSoa.p_color_start         = std::make_unique<glm::vec3[]>(max_particles_total);
  particleSoa.p_opacity_start       = std::make_unique<float[]>(max_particles_total);

  for (uint32_t i = 0; i < count; ++i) {
    soa.free_slots.push_back(i);
  }
}

int LvsEffectManager::initializeEffect(LvsEffects::effectProperties effect) {

  // --- Input clamping / validation ---
  // Emission
  effect.emission_radius           = std::max(0.f, effect.emission_radius);
  effect.emission_arc              = std::clamp(effect.emission_arc, 0.f, 360.f);
  effect.spawn_rate                = std::max(0.f, effect.spawn_rate);
  effect.max_presistent_particles  = std::max(1, effect.max_presistent_particles);
  effect.active = true;

  // Lifetime
  if (effect.particle_duration != -1.f)
    effect.particle_duration = std::max(0.f, effect.particle_duration);
  effect.particle_duration_variance = std::max(0.f, effect.particle_duration_variance);
  effect.particle_delay_variance    = std::max(0.f, effect.particle_delay_variance);
  effect.fade_in_time               = std::max(0.f, effect.fade_in_time);
  effect.fade_out_time              = std::max(0.f, effect.fade_out_time);

  // Appearance — opacity [0, 1]
  effect.particle_opacity_start = std::clamp(effect.particle_opacity_start, 0.f, 1.f);
  effect.particle_opacity_end   = std::clamp(effect.particle_opacity_end,   0.f, 1.f);

  // Appearance — color channels [0, 1]
  effect.particle_color_start  = glm::clamp(effect.particle_color_start,  0.f, 1.f);
  effect.particle_color_end    = glm::clamp(effect.particle_color_end,    0.f, 1.f);
  effect.particle_color2_start = glm::clamp(effect.particle_color2_start, 0.f, 1.f);
  effect.particle_color2_end   = glm::clamp(effect.particle_color2_end,   0.f, 1.f);

  // Physics
  effect.drag = std::max(0.f, effect.drag);
  effect.particle_velocity_start = std::max(0.f, effect.particle_velocity_start);

  // Looping / Repetition
  effect.loop_delay      = std::max(0.f, effect.loop_delay);
  effect.burst_count     = std::max(1,   effect.burst_count);
  effect.burst_interval  = std::max(0.f, effect.burst_interval);
  effect.loop_delay_reamining = effect.loop_delay;
  effect.spawn_accumulator = 0.f;
  effect.burst_timer       = effect.burst_interval; // prime so first burst fires on frame 1

  // Variance
  effect.velocity_variance         = std::max(0.f, effect.velocity_variance);
  effect.direction_variance        = std::max(0.f, effect.direction_variance);
  effect.angular_velocity_variance = std::max(0.f, effect.angular_velocity_variance);
  effect.opacity_variance          = std::max(0.f, effect.opacity_variance);

  int idx = soa.free_slots.back();
  soa.free_slots.pop_back();
  soa.active_indices.push_back(idx);

  syncPropertiesWithSoA(idx, effect, true);

  int particleCap = calculateMaxPresistentParticles(effect);
  soa.effect_max_simultaneous_particles[idx] = particleCap;
  soa.effect_particle_pool_effect[idx]       = static_cast<int>(m_ParticlePoolCursor);
  m_ParticlePoolCursor += static_cast<uint32_t>(particleCap);

  m_ParticleFreeSlots[idx].resize(particleCap);
  for (int i = 0; i < particleCap; ++i) {
    m_ParticleFreeSlots[idx][i] = i;
  }

  return idx;
}

void LvsEffectManager::pauseEffect(int idx) {
  if (idx < 0 || idx >= (int)m_MaxEffects) return;
  soa.effect_active[idx] = false;
}

void LvsEffectManager::continueEffect(int idx) {
  if (idx < 0 || idx >= (int)m_MaxEffects) return;
  soa.effect_active[idx] = true;
}

void LvsEffectManager::deleteEffect(int idx) {
  if (idx < 0 || idx >= (int)m_MaxEffects) return;

  int pool_base = soa.effect_particle_pool_effect[idx];
  int pool_cap  = soa.effect_max_simultaneous_particles[idx];

  m_ParticleFreeSlots[idx].clear();
  for (int local = 0; local < pool_cap; ++local) {
    particleSoa.p_alive[pool_base + local] = false;
    m_ParticleFreeSlots[idx].push_back(local);
  }

  auto& ai = soa.active_indices;
  ai.erase(std::remove(ai.begin(), ai.end(), idx), ai.end());

  soa.free_slots.push_back(idx);
  soa.effect_active[idx] = false;
}

LvsEffects::effectProperties LvsEffectManager::getEffectProperties(int idx) {
  validateEffectIndex(idx, "LvsEffectManager::getEffectProperties");
  LvsEffects::effectProperties props;
  syncPropertiesWithSoA(idx, props, false);
  return props;
}

template<typename T>
T LvsEffectManager::getEffectProperties(int idx, T LvsEffects::effectProperties::* field) {
  validateEffectIndex(idx, "LvsEffectManager::getEffectProperties");
  LvsEffects::effectProperties props;
  syncPropertiesWithSoA(idx, props, false);
  return props.*field;
}

template float       LvsEffectManager::getEffectProperties<float>(int, float LvsEffects::effectProperties::*);
template int         LvsEffectManager::getEffectProperties<int>(int, int LvsEffects::effectProperties::*);
template bool        LvsEffectManager::getEffectProperties<bool>(int, bool LvsEffects::effectProperties::*);
template glm::vec2   LvsEffectManager::getEffectProperties<glm::vec2>(int, glm::vec2 LvsEffects::effectProperties::*);
template glm::vec3   LvsEffectManager::getEffectProperties<glm::vec3>(int, glm::vec3 LvsEffects::effectProperties::*);
template uint32_t   LvsEffectManager::getEffectProperties<uint32_t>(int, uint32_t LvsEffects::effectProperties::*);
template LvsEasingFunctions::EaseType LvsEffectManager::getEffectProperties<LvsEasingFunctions::EaseType>(int, LvsEasingFunctions::EaseType LvsEffects::effectProperties::*);
template LvsGameObject* LvsEffectManager::getEffectProperties<LvsGameObject*>(int, LvsGameObject* LvsEffects::effectProperties::*);

void LvsEffectManager::updateEffect(int effect_idx, float dt) {
  if (!soa.effect_active[effect_idx]) return;

  // Loop delay countdown
  if (soa.effect_loop_delay_remaining[effect_idx] > 0.f) {
    soa.effect_loop_delay_remaining[effect_idx] -= dt;
    if (soa.effect_loop_delay_remaining[effect_idx] <= 0.f) {
      // Delay finished: prime timer so spawn fires immediately next tick
      soa.effect_burst_timer[effect_idx]       = soa.effect_burst_interval[effect_idx];
      soa.effect_spawn_accumulator[effect_idx] = 0.f;
    }
    return;
  }

  // --- Spawn ---
  if (soa.effect_burst_mode[effect_idx]) {
    soa.effect_burst_timer[effect_idx] += dt;
    float interval = soa.effect_burst_interval[effect_idx];
    bool fire = (interval == 0.f) ? (soa.effect_burst_timer[effect_idx] > 0.f)
                                  : (soa.effect_burst_timer[effect_idx] >= interval);
    if (fire) {
      for (int i = 0; i < soa.effect_burst_count[effect_idx]; ++i)
        spawnParticle(effect_idx);

      // Prevent re-fire this cycle; sentinel chosen so timer + dt won't reach interval again
      soa.effect_burst_timer[effect_idx] = -std::numeric_limits<float>::max();

      soa.effect_current_repetition[effect_idx]++;
      int rep = soa.effect_repetition[effect_idx];
      if (rep != -1 && soa.effect_current_repetition[effect_idx] >= rep) {
        soa.effect_active[effect_idx] = false;
        if (soa.effect_on_effect_finish[effect_idx])
          soa.effect_on_effect_finish[effect_idx](soa.effect_callback_data[effect_idx]);
        return;
      }
      soa.effect_loop_delay_remaining[effect_idx] = soa.effect_loop_delay[effect_idx];
      if (soa.effect_loop_delay_remaining[effect_idx] <= 0.f) {
        // No delay: reset timer immediately so next burst fires on schedule
        soa.effect_burst_timer[effect_idx] = interval;
      }
    }
  } else {
    if (soa.effect_spawn_rate[effect_idx] > 0.f) {
      soa.effect_spawn_accumulator[effect_idx] += dt;
      float interval = 1.f / soa.effect_spawn_rate[effect_idx];
      while (soa.effect_spawn_accumulator[effect_idx] >= interval) {
        soa.effect_spawn_accumulator[effect_idx] -= interval;
        spawnParticle(effect_idx);
      }
    }
  }

  // --- Per-particle update + death ---
  int pool_base = soa.effect_particle_pool_effect[effect_idx];
  int pool_cap  = soa.effect_max_simultaneous_particles[effect_idx];
  for (int local = 0; local < pool_cap; ++local) {
    int abs = pool_base + local;
    if (!particleSoa.p_alive[abs]) continue;

    if (particleSoa.p_delay_remaining[abs] > 0.f) {
      particleSoa.p_delay_remaining[abs] -= dt;
      continue;
    }

    particleSystem.updateParticlePosition(abs, dt);

    float lifetime = particleSoa.p_lifetime[abs];
    if (lifetime != -1.f && particleSoa.p_age[abs] >= lifetime) {
      particleSoa.p_alive[abs] = false;
      m_ParticleFreeSlots[effect_idx].push_back(local);
      if (soa.effect_on_particle_death[effect_idx])
        soa.effect_on_particle_death[effect_idx](soa.effect_callback_data[effect_idx]);
    }
  }
}

void LvsEffectManager::updateEffects(float dt) {
  for (int idx : soa.active_indices) {
    updateEffect(idx, dt);
  }
}

// ============================================================
//  PRIVATE METHODS / HELPERS
// ============================================================

void LvsEffectManager::syncPropertiesWithSoA(int idx, LvsEffects::effectProperties &props, bool writeToSOA) {
  #define SYNC_VAL(field, vector) if(writeToSOA) vector[idx] = field; else field = static_cast<decltype(field)>(vector[idx]);

  // --- General ---
  SYNC_VAL(props.EASE, soa.effect_EASE);

  // --- Particle reference ---
  if (writeToSOA) {
    soa.effect_particle[idx] = props.particle;
  } else {
    props.particle = soa.effect_particle[idx];
  }

  // --- Spawn / Emission ---
  SYNC_VAL(props.particle_ending_position,  soa.effect_particle_ending_position);
  SYNC_VAL(props.emission_radius,           soa.effect_emission_radius);
  SYNC_VAL(props.emission_arc,              soa.effect_emission_arc);
  SYNC_VAL(props.emission_arc_offset,       soa.effect_emission_arc_offset);
  SYNC_VAL(props.emit_from_edge,            soa.effect_emit_from_edge);
  SYNC_VAL(props.spawn_rate,                soa.effect_spawn_rate);
  SYNC_VAL(props.repetition,                soa.effect_repetition);
  SYNC_VAL(props.reverse_on_finish,         soa.effect_reverse_on_finish);
  SYNC_VAL(props.max_presistent_particles,  soa.effect_max_presistent_particles);
  SYNC_VAL(props.active,                    soa.effect_active);
  SYNC_VAL(props.spawn_accumulator,         soa.effect_spawn_accumulator);

  // --- Motion / Physics ---
  SYNC_VAL(props.particle_velocity_start,          soa.effect_particle_velocity_start);
  SYNC_VAL(props.particle_velocity_end,            soa.effect_particle_velocity_end);
  SYNC_VAL(props.particle__acceleration,           soa.effect_particle__acceleration);
  SYNC_VAL(props.gravity_strength,                 soa.effect_gravity_strength);
  SYNC_VAL(props.particle_direction_end,           soa.effect_particle_direction_end);
  SYNC_VAL(props.particle_angular_velocity,        soa.effect_particle_angular_velocity);
  SYNC_VAL(props.particle_angular_velocity_end,    soa.effect_particle_angular_velocity_end);
  SYNC_VAL(props.drag,                             soa.effect_drag);
  SYNC_VAL(props.velocity_ease,                    soa.effect_velocity_ease);
  SYNC_VAL(props.velocity_custom_ease_function,    soa.effect_velocity_custom_ease_function);

  // --- Lifetime ---
  SYNC_VAL(props.particle_duration,          soa.effect_particle_duration);
  SYNC_VAL(props.particle_duration_variance, soa.effect_particle_duration_variance);
  SYNC_VAL(props.particle_delay_variance,    soa.effect_particle_delay_variance);
  SYNC_VAL(props.fade_in_time,               soa.effect_fade_in_time);
  SYNC_VAL(props.fade_out_time,              soa.effect_fade_out_time);
  SYNC_VAL(props.fade_ease,                  soa.effect_fade_ease);
  SYNC_VAL(props.destroy_on_finish,          soa.effect_destroy_on_finish);

  // --- Appearance ---
  SYNC_VAL(props.particle_color_start,       soa.effect_particle_color_start);
  SYNC_VAL(props.particle_color_end,         soa.effect_particle_color_end);
  SYNC_VAL(props.particle_color2_start,      soa.effect_particle_color2_start);
  SYNC_VAL(props.particle_color2_end,        soa.effect_particle_color2_end);
  SYNC_VAL(props.color_ease,                 soa.effect_color_ease);
  SYNC_VAL(props.use_gradient,               soa.effect_use_gradient);
  SYNC_VAL(props.gradient_direction_start,   soa.effect_gradient_direction_start);
  SYNC_VAL(props.gradient_direction_end,     soa.effect_gradient_direction_end);
  SYNC_VAL(props.particle_opacity_start,     soa.effect_particle_opacity_start);
  SYNC_VAL(props.particle_opacity_end,       soa.effect_particle_opacity_end);
  SYNC_VAL(props.particle_scale_end,         soa.effect_particle_scale_end);
  SYNC_VAL(props.scale_ease,                 soa.effect_scale_ease);
  SYNC_VAL(props.color_custom_ease_function, soa.effect_color_custom_ease_function);
  SYNC_VAL(props.scale_custom_ease_function, soa.effect_scale_custom_ease_function);

  // --- Looping / Repetition ---
  SYNC_VAL(props.loop_delay,           soa.effect_loop_delay);
  SYNC_VAL(props.burst_mode,           soa.effect_burst_mode);
  SYNC_VAL(props.burst_count,          soa.effect_burst_count);
  SYNC_VAL(props.burst_interval,       soa.effect_burst_interval);
  SYNC_VAL(props.burst_timer,          soa.effect_burst_timer);
  SYNC_VAL(props.current_repetition,   soa.effect_current_repetition);
  SYNC_VAL(props.loop_delay_reamining, soa.effect_loop_delay_remaining);

  // --- Randomness / Variance ---
  SYNC_VAL(props.random_seed,                  soa.effect_random_seed);
  SYNC_VAL(props.velocity_variance,            soa.effect_velocity_variance);
  SYNC_VAL(props.direction_variance,           soa.effect_direction_variance);
  SYNC_VAL(props.angular_velocity_variance,    soa.effect_angular_velocity_variance);
  SYNC_VAL(props.scale_variance,               soa.effect_scale_variance);
  SYNC_VAL(props.color_start_variance,         soa.effect_color_start_variance);
  SYNC_VAL(props.color_end_variance,           soa.effect_color_end_variance);
  SYNC_VAL(props.opacity_variance,             soa.effect_opacity_variance);
  SYNC_VAL(props.starting_position_variance,   soa.effect_starting_position_variance);

  // --- Callbacks ---
  SYNC_VAL(props.callback_data,               soa.effect_callback_data);
  SYNC_VAL(props.on_effect_finish,            soa.effect_on_effect_finish);
  SYNC_VAL(props.on_particle_spawn,           soa.effect_on_particle_spawn);
  SYNC_VAL(props.on_particle_death,           soa.effect_on_particle_death);
  SYNC_VAL(props.EFFECT_CUSTOM_EASE_FUNCTION, soa.effect_EFFECT_CUSTOM_EASE_FUNCTION);

  #undef SYNC_VAL
}

int LvsEffectManager::calculateMaxPresistentParticles(LvsEffects::effectProperties &props) {
  int result = 1;

  float max_lifetime = props.particle_duration
                      + props.particle_duration_variance
                      + props.particle_delay_variance;

  if (!props.burst_mode && props.particle_duration != -1.f) {
    result = (int)std::ceil(props.spawn_rate * max_lifetime * 1.2f);
  } else if (!props.burst_mode && props.particle_duration == -1.f) {
    result = props.max_presistent_particles;
  } else if (props.burst_mode && props.particle_duration != -1.f) {
    int overlapping_bursts;
    if (props.burst_interval >= max_lifetime || props.repetition == 1) {
      overlapping_bursts = 1;
    } else {
      overlapping_bursts = (int)std::ceil(max_lifetime / props.burst_interval);
      if (props.repetition > 0) { overlapping_bursts = std::min(overlapping_bursts, props.repetition); }
    }
    result = props.burst_count * overlapping_bursts;
  } else if (props.burst_mode && props.particle_duration == -1.f && props.repetition > 0) {
    result = props.burst_count * props.repetition;
  } else {
    result = props.max_presistent_particles;
  }

  result = std::max(1, result);
  return result;
}

void LvsEffectManager::spawnParticle(int effect_idx) {
  if (m_ParticleFreeSlots[effect_idx].empty()) return;
  int local_slot = m_ParticleFreeSlots[effect_idx].back();
  m_ParticleFreeSlots[effect_idx].pop_back();
  particleSystem.initalizeParticle(effect_idx, local_slot);
}

void LvsEffectManager::validateEffectIndex(int idx, const char* callerName) const {
  if (idx < 0 || idx >= (int)m_MaxEffects) {
    throw std::runtime_error(cpc::Red + "[" + callerName + "] idx (" + std::to_string(idx) +
      ") is out of range for the effect pool (valid range is 0 to " + std::to_string(m_MaxEffects - 1) +
      "). Fix: pass a valid SoA slot index returned by LvsEffectManager::initializeEffect()." + cpc::Reset);
  }

  bool isActive = std::find(soa.active_indices.begin(), soa.active_indices.end(), idx) != soa.active_indices.end();
  if (!isActive) {
    std::cerr << cpc::Yellow << "[" << callerName << "] idx (" << idx << ") is within range but is not a "
      "currently active effect slot (it may have been deleted, or initializeEffect() was never called for it). "
      "The returned value will reflect stale or default data. Fix: verify idx was returned by a successful "
      "initializeEffect() call and has not since been deleted." << cpc::Reset << std::endl;
  }
}

} // namespace lvs
