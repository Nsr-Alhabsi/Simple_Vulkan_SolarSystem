#include "lvs_effect_manager.hpp"
#include "core/lvs_effects.hpp"
#include "../ADDONS/cp_color.hpp"

namespace lvs {

void LvsEffectManager::init(uint32_t count) {
  m_MaxEffects = count;

  soa.effect_durations = std::make_unique<float[]>(count);
  soa.effect_delays = std::make_unique<float[]>(count);

  soa.effect_ease_types = std::make_unique<LvsEasingFunctions::EaseType[]>(count);
  soa.effect_particle_amounts = std::make_unique<uint32_t[]>(count);

  soa.effect_elapsed_times = std::make_unique<float[]>(count);
  soa.effect_elapsed_delay_times = std::make_unique<float[]>(count);

  soa.effect_delays_finished = std::make_unique<bool[]>(count);
  soa.effect_particles = std::make_unique<LvsGameObject[]>(count);

  soa.m_ActiveIndices = std::make_unique<bool[]>(count);
  std::fill_n(soa.m_ActiveIndices.get(), count, false);

  soa.m_FreeIndices.reserve(count);
  for (uint32_t i = 0; i < count; i++) {
    soa.m_FreeIndices.push_back(i);
    
  }
}

int LvsEffectManager::initializeEffect(LvsEffects::effectProperties effect) {
  if (effect.duration <= 0.f) {
    throw std::runtime_error("INVALID EFFECT: Effect duration must be greater than 0");
  }

  if (effect.particle == nullptr) {
    std::cout << cpc::Yellow << "WARNING: Effect particle is null, defaulting to a square" << cpc::Reset << std::endl;
    auto squareParticle = &LvsGameObject::createGameObject(LvsGameObject::ObjectType::Square, lvsDevice);
    squareParticle->color = {1.f, 1.f, 1.f};
    squareParticle->transform2D.scale /= 10.f;
    effect.particle = squareParticle;
  }

  
}

}