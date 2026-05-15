#pragma once

#include "core/lvs_effects.hpp"
#include "core/lvs_SoA_effects.hpp"

namespace lvs {

class LvsEffectManager {
private:
  uint32_t m_MaxEffects;

  void syncPropertiesWithSoA(int idx, LvsEffects::effectProperties &props, bool writeToSOA);

public:
  void init(uint32_t count);
  int initializeEffect(LvsEffects::effectProperties effect);

  LvsSOAEffects soa;
};

}