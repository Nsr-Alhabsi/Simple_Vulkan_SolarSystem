#include "lvs_effect_manager.hpp"
#include "core/lvs_effects.hpp"
#include "../ADDONS/cp_color.hpp"
#include <iostream>
#include <algorithm>
namespace lvs {

void LvsEffectManager::init(uint32_t count) {
  m_MaxEffects = count;

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

  // --- Motion / Physics ---
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
  soa.effect_current_repetition = std::make_unique<int[]>(count);

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

  for (uint32_t i = 0; i < count; ++i) {
    soa.free_slots.push_back(i);
  }
}

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
  SYNC_VAL(props.particle_ending_position, soa.effect_particle_ending_position);
  SYNC_VAL(props.emission_radius,          soa.effect_emission_radius);
  SYNC_VAL(props.emission_arc,             soa.effect_emission_arc);
  SYNC_VAL(props.emission_arc_offset,      soa.effect_emission_arc_offset);
  SYNC_VAL(props.emit_from_edge,           soa.effect_emit_from_edge);
  SYNC_VAL(props.spawn_rate,               soa.effect_spawn_rate);
  SYNC_VAL(props.repetition,               soa.effect_repetition);
  SYNC_VAL(props.reverse_on_finish,        soa.effect_reverse_on_finish);

  // --- Motion / Physics ---
  SYNC_VAL(props.particle_velocity_end,            soa.effect_particle_velocity_end);
  SYNC_VAL(props.particle__acceleration,            soa.effect_particle__acceleration);
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
  SYNC_VAL(props.loop_delay,          soa.effect_loop_delay);
  SYNC_VAL(props.burst_mode,          soa.effect_burst_mode);
  SYNC_VAL(props.burst_count,         soa.effect_burst_count);
  SYNC_VAL(props.burst_interval,      soa.effect_burst_interval);
  SYNC_VAL(props.current_repetition,  soa.effect_current_repetition);

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
  SYNC_VAL(props.callback_data,              soa.effect_callback_data);
  SYNC_VAL(props.on_effect_finish,           soa.effect_on_effect_finish);
  SYNC_VAL(props.on_particle_spawn,          soa.effect_on_particle_spawn);
  SYNC_VAL(props.on_particle_death,          soa.effect_on_particle_death);
  SYNC_VAL(props.EFFECT_CUSTOM_EASE_FUNCTION, soa.effect_EFFECT_CUSTOM_EASE_FUNCTION);

  #undef SYNC_VAL
}

int LvsEffectManager::initializeEffect(LvsEffects::effectProperties effect) {

  // --- Input clamping / validation ---
  // Emission
  effect.emission_radius = std::max(0.f, effect.emission_radius);
  effect.emission_arc    = std::clamp(effect.emission_arc, 0.f, 360.f);
  effect.spawn_rate      = std::max(0.f, effect.spawn_rate);

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

  // Looping / Repetition
  effect.loop_delay      = std::max(0.f, effect.loop_delay);
  effect.burst_count     = std::max(1,   effect.burst_count);
  effect.burst_interval  = std::max(0.f, effect.burst_interval);

  // Variance
  effect.velocity_variance         = std::max(0.f, effect.velocity_variance);
  effect.direction_variance        = std::max(0.f, effect.direction_variance);
  effect.angular_velocity_variance = std::max(0.f, effect.angular_velocity_variance);
  effect.opacity_variance          = std::max(0.f, effect.opacity_variance);

  int idx = soa.free_slots.back();
  soa.free_slots.pop_back();
  soa.active_indices.push_back(idx);

  syncPropertiesWithSoA(idx, effect, true);

  return idx;
}

template<typename T>
T LvsEffectManager::getEffectProperties(int idx, T LvsEffects::effectProperties::* field) {
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

}