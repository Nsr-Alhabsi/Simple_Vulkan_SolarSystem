#pragma once

#include "core/lvs_effects.hpp"
#include "core/lvs_SoA_effects.hpp"
#include "lvs_particle_system.hpp"

#include "../core/lvs_device.hpp"

#include <random>

namespace lvs {
class LvsEffectManager {
private:
  uint32_t m_MaxEffects;
  uint32_t m_MaxParticles;
  uint32_t m_ParticlePoolCursor;
  LvsSOAParticles particleSoa;
  std::vector<std::vector<int>> m_ParticleFreeSlots;
  std::mt19937 m_Rng;

  int calculateMaxPresistentParticles(LvsEffects::effectProperties &props);
  void spawnParticle(int effect_idx);

  void syncPropertiesWithSoA(int idx, LvsEffects::effectProperties &props, bool writeToSOA);
  LvsSOAEffects soa;
  LvsDevice& lvsDevice;
  LvsParticleSystem particleSystem;
public:
  LvsEffectManager(LvsDevice& device)
    : lvsDevice{device}, m_Rng{std::random_device{}()}, particleSystem{soa, particleSoa, m_Rng} {};

  void init(uint32_t count, uint32_t max_particles_total);
  int initializeEffect(LvsEffects::effectProperties effect);

  void updateEffect(int effect_idx, float dt);
  void updateEffects(float dt);

  const LvsSOAEffects&   getEffectSoA()   const { return soa; }
  const LvsSOAParticles& getParticleSoA() const { return particleSoa; }

  template<typename T>
  T getEffectProperties(int idx, T LvsEffects::effectProperties::* field);
};

}