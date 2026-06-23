#pragma once

#include "core/lvs_effects.hpp"
#include "core/lvs_SoA_effects.hpp"
#include "../core/lvs_device.hpp"
namespace lvs {
class LvsEffectManager {
private:
  uint32_t m_MaxEffects;

  void syncPropertiesWithSoA(int idx, LvsEffects::effectProperties &props, bool writeToSOA);
  
  LvsSOAEffects soa;
  LvsDevice& lvsDevice;
public:
  LvsEffectManager(LvsDevice& device) : lvsDevice{device} {};

  void init(uint32_t count);
  int initializeEffect(LvsEffects::effectProperties effect);

  template<typename T>
  T getEffectProperties(int idx, T LvsEffects::effectProperties::* field);
};

}