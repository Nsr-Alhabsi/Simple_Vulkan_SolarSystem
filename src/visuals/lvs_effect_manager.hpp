#pragma once

#include "core/lvs_effects.hpp"
#include "core/lvs_SoA_effects.hpp"
#include "../core/lvs_device.hpp"

namespace lvs {

class LvsEffectManager {
private:
  uint32_t m_MaxEffects;

public:
  void init(uint32_t count);
  int initializeEffect(LvsEffects::effectProperties effect);

  LvsSOAEffects soa;
  LvsDevice& lvsDevice;
};

}