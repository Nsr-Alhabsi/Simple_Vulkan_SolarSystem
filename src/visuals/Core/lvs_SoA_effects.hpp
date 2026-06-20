#pragma once

#include "../../utils/easing_functions.hpp"
#include "../../models/lvs_game_object.hpp"

// std:
#include <memory>
#include <vector>

namespace lvs {

struct LvsSOAEffects {
  std::unique_ptr<LvsEasingFunctions::EaseType[]> effect_ease_types;
  std::unique_ptr<uint32_t[]> effect_particle_amounts;
  
  std::unique_ptr<float[]> effect_durations;
  std::unique_ptr<float[]> effect_delays;

  std::unique_ptr<float[]> effect_elapsed_times;
  std::unique_ptr<float[]> effect_elapsed_delay_times;

  std::unique_ptr<bool[]> effect_delays_finished;
  std::unique_ptr<LvsGameObject[]> effect_particles;

  std::unique_ptr<glm::vec2[]> effect_particle_starting_positions;
  std::unique_ptr<glm::vec2[]> effect_particle_scales;
  std::unique_ptr<float[]> effect_particle_directions;
  std::unique_ptr<float[]> effect_particle_velocities;

  std::unique_ptr<bool[]> effect_effected_by_gravity;

  std::vector<int> active_indices;
  std::vector<int> free_slots;
};

}