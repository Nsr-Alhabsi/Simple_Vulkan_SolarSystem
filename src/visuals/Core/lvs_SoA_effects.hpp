#pragma once

#include "../../utils/easing_functions.hpp"
#include "../../models/lvs_game_object.hpp"

#include <glm/glm.hpp>

// std:
#include <memory>
#include <vector>

namespace lvs {

struct LvsSOAEffects {
  // --- General ---
  std::unique_ptr<LvsEasingFunctions::EaseType[]> effect_EASE;

  // --- Particle reference ---
  std::unique_ptr<LvsGameObject*[]> effect_particle;

  // --- Spawn / Emission ---
  std::unique_ptr<glm::vec2[]> effect_particle_ending_position;
  std::unique_ptr<float[]>     effect_emission_radius;
  std::unique_ptr<float[]>     effect_emission_arc;
  std::unique_ptr<float[]>     effect_emission_arc_offset;
  std::unique_ptr<bool[]>      effect_emit_from_edge;
  std::unique_ptr<float[]>     effect_spawn_rate;
  std::unique_ptr<int[]>       effect_repetition;
  std::unique_ptr<bool[]>      effect_reverse_on_finish;

  // --- Motion / Physics ---
  std::unique_ptr<float[]>     effect_particle_velocity_end;
  std::unique_ptr<glm::vec2[]> effect_particle__acceleration;
  std::unique_ptr<float[]>     effect_gravity_strength;
  std::unique_ptr<float[]>     effect_particle_direction_end;
  std::unique_ptr<float[]>     effect_particle_angular_velocity;
  std::unique_ptr<float[]>     effect_particle_angular_velocity_end;
  std::unique_ptr<float[]>     effect_drag;
  std::unique_ptr<LvsEasingFunctions::EaseType[]> effect_velocity_ease;
  std::unique_ptr<float(*[])(float)>              effect_velocity_custom_ease_function;

  // --- Lifetime ---
  std::unique_ptr<float[]> effect_particle_duration;
  std::unique_ptr<float[]> effect_particle_duration_variance;
  std::unique_ptr<float[]> effect_particle_delay_variance;
  std::unique_ptr<float[]> effect_fade_in_time;
  std::unique_ptr<float[]> effect_fade_out_time;
  std::unique_ptr<LvsEasingFunctions::EaseType[]> effect_fade_ease;
  std::unique_ptr<bool[]>  effect_destroy_on_finish;

  // --- Appearance ---
  std::unique_ptr<glm::vec3[]> effect_particle_color_start;
  std::unique_ptr<glm::vec3[]> effect_particle_color_end;
  std::unique_ptr<glm::vec3[]> effect_particle_color2_start;
  std::unique_ptr<glm::vec3[]> effect_particle_color2_end;
  std::unique_ptr<LvsEasingFunctions::EaseType[]> effect_color_ease;
  std::unique_ptr<bool[]>      effect_use_gradient;
  std::unique_ptr<glm::vec2[]> effect_gradient_direction_start;
  std::unique_ptr<glm::vec2[]> effect_gradient_direction_end;
  std::unique_ptr<float[]>     effect_particle_opacity_start;
  std::unique_ptr<float[]>     effect_particle_opacity_end;
  std::unique_ptr<glm::vec2[]> effect_particle_scale_end;
  std::unique_ptr<LvsEasingFunctions::EaseType[]> effect_scale_ease;
  std::unique_ptr<float(*[])(float)>              effect_color_custom_ease_function;
  std::unique_ptr<float(*[])(float)>              effect_scale_custom_ease_function;

  // --- Looping / Repetition ---
  std::unique_ptr<float[]> effect_loop_delay;
  std::unique_ptr<bool[]>  effect_burst_mode;
  std::unique_ptr<int[]>   effect_burst_count;
  std::unique_ptr<float[]> effect_burst_interval;
  std::unique_ptr<int[]>   effect_current_repetition;

  // --- Randomness / Variance ---
  std::unique_ptr<uint32_t[]>  effect_random_seed;
  std::unique_ptr<float[]>     effect_velocity_variance;
  std::unique_ptr<float[]>     effect_direction_variance;
  std::unique_ptr<float[]>     effect_angular_velocity_variance;
  std::unique_ptr<glm::vec2[]> effect_scale_variance;
  std::unique_ptr<glm::vec3[]> effect_color_start_variance;
  std::unique_ptr<glm::vec3[]> effect_color_end_variance;
  std::unique_ptr<float[]>     effect_opacity_variance;
  std::unique_ptr<glm::vec2[]> effect_starting_position_variance;

  // --- Callbacks ---
  std::unique_ptr<void*[]>          effect_callback_data;
  std::unique_ptr<void(*[])(void*)> effect_on_effect_finish;
  std::unique_ptr<void(*[])(void*)> effect_on_particle_spawn;
  std::unique_ptr<void(*[])(void*)> effect_on_particle_death;
  std::unique_ptr<float(*[])(float)>              effect_EFFECT_CUSTOM_EASE_FUNCTION;

  // --- Pool bookkeeping (unchanged) ---
  std::vector<int> active_indices;
  std::vector<int> free_slots;
};

}
