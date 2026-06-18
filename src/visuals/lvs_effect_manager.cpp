#include "lvs_effect_manager.hpp"
#include "core/lvs_effects.hpp"
#include "../ADDONS/cp_color.hpp"
#include <iostream>

namespace lvs {

void LvsEffectManager::init(LvsDevice& device, uint32_t count) {
  m_MaxEffects = count;
  m_Device = &device;

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
  if (effect.duration <= 0.f || effect.delay <= 0.f) {
    std::cout << cpc::Yellow << "ERROR: Effect duration or delay must be greater than zero, Defaulting to 1" << cpc::Reset << std::endl;
    
    effect.duration = effect.duration <= 0.f ? 1.f : effect.duration;
    effect.delay = effect.delay <= 0.f ? 1.f : effect.delay;
  }

  if (effect.particle == nullptr) {
    std::cout << cpc::Yellow << "EFFECT WARNING: Particle was set to nullptr deafulting to block" << cpc::Reset << std::endl;
    auto block = LvsGameObject::createGameObject(LvsGameObject::ObjectType::Square, *m_Device);
    block.color = {1.f, 1.f, 1.f};
    block.transform2D.scale /= 10;

    effect.particle = &block;
  }

  if (soa.free_slots.empty()) {
    std::cout << cpc::Red << "No free slots found" << cpc::Reset << std::endl;
    return -1; // No free slots
  }

  effect.elapsed_time = 0.f;
  effect.elapsed_delay_time = 0.f;
  effect.delay_finished = effect.delay == 0.f ? true : false;

  int idx = soa.free_slots.back();
  soa.free_slots.pop_back();
  soa.active_indices.push_back(idx);

  syncPropertiesWithSoA(idx, effect, true);

  return idx;
}

}