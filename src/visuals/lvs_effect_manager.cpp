#include "lvs_effect_manager.hpp"
#include "core/lvs_effects.hpp"
#include "../ADDONS/cp_color.hpp"
#include <iostream>

namespace lvs {

void LvsEffectManager::init(uint32_t count) {
  m_MaxEffects = count;

  soa.effect_ease_types = std::make_unique<LvsEasingFunctions::EaseType[]>(count);
  soa.effect_particle_amounts = std::make_unique<uint32_t[]>(count);
  soa.effect_durations = std::make_unique<float[]>(count);
  soa.effect_delays = std::make_unique<float[]>(count);
  soa.effect_elapsed_times = std::make_unique<float[]>(count);
  soa.effect_elapsed_delay_times = std::make_unique<float[]>(count);
  soa.effect_delays_finished = std::make_unique<bool[]>(count);
  soa.effect_particles = std::make_unique<LvsGameObject[]>(count);

  for (uint32_t i = 0; i < count; ++i) {
    soa.free_slots.push_back(i);
  }
}

void LvsEffectManager::syncPropertiesWithSoA(int idx, LvsEffects::effectProperties &props, bool writeToSOA) {
  #define SYNC_VAL(field, vector) if(writeToSOA) vector[idx] = field; else field = static_cast<decltype(field)>(vector[idx]);

  /// @brief Easing curve applied to the overall effect's main interpolation.
    /// @note Defaults to LINEAR. Applies globally unless overridden by a per-property ease field.
    LvsEasingFunctions::EaseType EASE{LvsEasingFunctions::LINEAR};

    // PARTICLE PROPERTIES

    /// @brief Pointer to the game object used as the particle template for this effect.
    /// @note Must not be null before the effect is played. Ownership is not transferred.
    LvsGameObject* particle{nullptr};

    // Spawn / Emission

    /// @brief World-space position where particles travel toward (or end up at) over their lifetime.
    /// @note Units match the world coordinate system (NDC-derived or scene units).
    glm::vec2 particle_ending_position{0.f, 0.f};

    /// @brief Radius of the circular emission area around the emitter origin.
    /// @note 0 emits from a single point. Units are scene/world units.
    float emission_radius{0.f};

    /// @brief Angular span of the emission cone, in degrees.
    /// @note 360 emits in all directions. Values less than 360 create a directed arc.
    float emission_arc{360.f};

    /// @brief Rotational offset applied to the emission arc, in degrees.
    /// @note 0 means the arc is centered on the positive-X axis. Positive values rotate counter-clockwise.
    float emission_arc_offset{0.f};

    /// @brief When true, particles spawn on the outer edge of emission_radius rather than inside it.
    bool emit_from_edge{false};

    /// @brief Number of particles spawned per second.
    /// @note 0 disables continuous emission. Use burst_mode for instantaneous spawning.
    float spawn_rate{0.f};

    /// @brief Total number of times the effect replays after the first run.
    /// @note 1 means the effect plays once (no repetition). -1 loops indefinitely.
    int repetition{1};

    /// @brief When true, the effect plays in reverse on alternating repetitions (ping-pong looping).
    bool reverse_on_finish{false};

  SYNC_VAL(props.EASE, soa.effect_ease_types);

  if (writeToSOA) {
    if (props.particle) {
      soa.effect_particles[idx] = std::move(*props.particle);
    }
  } else {
    props.particle = &soa.effect_particles[idx];
  }

  #undef SYNC_VAL
}

int LvsEffectManager::initializeEffect(LvsEffects::effectProperties effect) {
  

  int idx = soa.free_slots.back();
  soa.free_slots.pop_back();
  soa.active_indices.push_back(idx);

  syncPropertiesWithSoA(idx, effect, true);

  return idx;
}

}