#pragma once

#include "core/lvs_effects.hpp"
#include "core/lvs_SoA_effects.hpp"

#include "../core/lvs_device.hpp"

#include "../vulkan_simulation.hpp"

namespace lvs {
class LvsEffectManager {
private:
  uint32_t m_MaxEffects;
  uint32_t m_MaxParticles;
  uint32_t m_ParticlePoolCursor;
  LvsSOAParticles particleSoa;
  std::vector<std::vector<int>> m_ParticleFreeSlots;

  int calculateMaxPresistentParticles(LvsEffects::effectProperties &props);

  void syncPropertiesWithSoA(int idx, LvsEffects::effectProperties &props, bool writeToSOA);
  void initalizeParticle(int effect_idx, int local_slot);

  LvsSOAEffects soa;
  LvsDevice& lvsDevice;
public:
  LvsEffectManager(LvsDevice& device) : lvsDevice{device} {};

  void init(uint32_t count, uint32_t max_particles_total);
  int initializeEffect(LvsEffects::effectProperties effect);

  template<typename T>
  T getEffectProperties(int idx, T LvsEffects::effectProperties::* field);
};

}