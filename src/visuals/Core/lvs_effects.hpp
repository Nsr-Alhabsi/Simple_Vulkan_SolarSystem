#pragma once

#include "../../utils/easing_functions.hpp"
#include "../../models/lvs_game_object.hpp"

#include <glm/glm.hpp>

namespace lvs {

class LvsEffectManager;

class LvsEffects {
public:
  struct effectProperties {
    LvsEasingFunctions::EaseType EASE{LvsEasingFunctions::LINEAR};

    // PARTICLE PROPERTIES

    LvsGameObject* particle{nullptr};
  
    // Spawn / Emission
    
    glm::vec2 particle_ending_position{0.f, 0.f};
    float emission_radius{0.f};
    float emission_arc{360.f};
    float emission_arc_offset{0.f};
    bool emit_from_edge{false};
    float spawn_rate{0.f};
    int repetition{1};
    bool reverse_on_finish{false};

    // Motion / Physics
    
    float particle_velocity_end{1.f};
    glm::vec2 particle__acceleration{0.f, 0.f};
    float gravity_strength{0.f};
    float particle_direction_end{0.f};
    float particle_angular_velocity{0.f};
    float particle_angular_velocity_end{0.f};
    float drag{0.f};
    LvsEasingFunctions::EaseType velocity_ease{LvsEasingFunctions::LINEAR};
    float (*velocity_custom_ease_function)(float){nullptr};

    // Lifetime
    
    float particle_duration{-1.f}; // per-particle lifetime; -1 = inherit duration
    float particle_duration_variance{0.f};
    float particle_delay_variance{0.f};
    float fade_in_time{0.f};
    float fade_out_time{0.f};
    LvsEasingFunctions::EaseType fade_ease{LvsEasingFunctions::LINEAR};
    bool destroy_on_finish{true};

    // Apperance
    glm::vec3 particle_color_start{1, 1, 1};
    glm::vec3 particle_color_end{1, 1, 1};
    glm::vec3 particle_color2_start{1, 1, 1};
    glm::vec3 particle_color2_end{1, 1, 1};
    LvsEasingFunctions::EaseType color_ease{LvsEasingFunctions::LINEAR};
    bool use_gradient{false};
    glm::vec2 gradient_direction_start{1.f, 0.f};
    glm::vec2 gradient_direction_end{1.f, 0.f};
    float particle_opacity_start{1.f};
    float particle_opacity_end{1.f}; //TODO: Make sure to add alpha color support in the shader
    glm::vec2 particle_scale_end{1, 1};
    LvsEasingFunctions::EaseType scale_ease{LvsEasingFunctions::LINEAR};
    float (*color_custom_ease_function)(float){nullptr};
    float (*scale_custom_ease_function)(float){nullptr};

    // Looping / Repetition
    float loop_delay{0.f};
    bool burst_mode{false};
    int burst_count{1};
    float burst_interval{0.f};
    int current_repetition{0};

    // Randomness / Variance

    uint32_t random_seed{0};
    float velocity_variance{0.f};
    float direction_variance{0.f};
    float angular_velocity_variance{0.f};
    glm::vec2 scale_variance{0.f, 0.f};
    glm::vec3 color_start_variance{0.f, 0.f, 0.f};
    glm::vec3 color_end_variance{0.f, 0.f, 0.f};
    float opacity_variance{0.f};
    glm::vec2 starting_position_variance{0.f, 0.f};

    // Callbacks

    void* callback_data{nullptr};
    void (*on_effect_finish)(void*){nullptr};
    void (*on_particle_spawn)(void*){nullptr};
    void (*on_particle_death)(void*){nullptr};
    float (*EFFECT_CUSTOM_EASE_FUNCTION)(float){nullptr};
  };

  LvsEffects(LvsEffectManager& manager) : m_Manager{manager} {}

  int setEffect(effectProperties effect);

  void updateEffects(); 
  
  void deleteEffect();
  
  void pauseEffect();
  
  void continueEffect();
private:
  LvsEffectManager& m_Manager;
};

}