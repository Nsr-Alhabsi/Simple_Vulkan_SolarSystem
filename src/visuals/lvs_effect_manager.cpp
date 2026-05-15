#include "lvs_effect_manager.hpp"
#include "core/lvs_effects.hpp"

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

  SYNC_VAL(props.EASE, soa.effect_ease_types);
  SYNC_VAL(props.particle_amount, soa.effect_particle_amounts);
  SYNC_VAL(props.duration, soa.effect_durations);
  SYNC_VAL(props.delay, soa.effect_delays);
  SYNC_VAL(props.elapsed_time, soa.effect_elapsed_times);
  SYNC_VAL(props.elapsed_delay_time, soa.effect_elapsed_delay_times);
  SYNC_VAL(props.delay_finished, soa.effect_delays_finished);

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
  if (soa.free_slots.empty()) {
    return -1; // No free slots
  }

  int idx = soa.free_slots.back();
  soa.free_slots.pop_back();
  soa.active_indices.push_back(idx);

  syncPropertiesWithSoA(idx, effect, true);

  return idx;
}

}