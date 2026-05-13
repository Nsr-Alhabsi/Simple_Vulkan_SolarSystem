#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

// Forward declarations or define enums here if they are shared
namespace lvs {

// Using integers or forward-declared enums to break circular dependency
class LvsSoAAnimations {
public:
  // We'll use int for enums to avoid circular dependency, or move enums to a base file
  std::vector<int> vector_animation_type;
  std::vector<int> vector_animation_ease;
  std::vector<int> vector_animation_state;

  std::vector<std::string> vector_animation_name;
  
  std::vector<int> vector_target_ID;
  
  std::vector<float> vector_animation_duration;
  std::vector<float> vector_animation_delay;
  std::vector<float> vector_animation_elapsed_time;
  std::vector<float> vector_animation_elapsed_delay_time;
  std::vector<int> vector_animation_repetition;
  
  std::vector<bool> vector_animation_delay_finished;
  std::vector<bool> vector_animation_reverse_on_finish;

  std::vector<float> vector_animation_starting_radian;
  std::vector<glm::vec2> vector_animation_pivot_point;
  std::vector<float> vector_animation_ending_radian;
  std::vector<float> vector_animation_radius;

  std::vector<glm::vec2> vector_animation_starting_position;
  std::vector<glm::vec2> vector_animation_ending_position;
  std::vector<bool> vector_animation_is_relative;

  std::vector<glm::vec2> vector_animation_starting_scale;
  std::vector<glm::vec2> vector_animation_ending_scale;

  std::vector<void*> vector_animation_callback;
  std::vector<float(*)(float)> vector_animation_custom_ease_function;

  std::vector<int> ANIMATION_SOA_VECTOR_ANIMATION_ACTIVE_INDICES;
  std::vector<int> ANIMATION_SOA_VECTOR_ANIMATION_FREE_SLOTS;
};

}