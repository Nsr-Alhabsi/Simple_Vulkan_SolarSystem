#pragma once

#include "Core/lvs_SoA_effects.hpp"

#include <random>

namespace lvs {

class LvsParticleSystem {
public:
  LvsParticleSystem(LvsSOAEffects& effectSoa, LvsSOAParticles& particleSoa, std::mt19937& rng)
    : m_EffectSoa{effectSoa}, m_ParticleSoa{particleSoa}, m_Rng{rng} {}

  void initalizeParticle(int effect_idx, int local_slot);
  void updateParticlePosition(int idx);

private:
  void updateParticleRotation(int idx);
  void updateParticleScale(int idx);
  void updateParticleTranslation(int idx);

  LvsSOAEffects&   m_EffectSoa;
  LvsSOAParticles& m_ParticleSoa;
  std::mt19937&    m_Rng;
};

}
