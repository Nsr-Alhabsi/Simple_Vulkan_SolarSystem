#pragma once

#include "../../utils/easing_functions.hpp"
#include "../../models/lvs_game_object.hpp"

namespace lvs {

class LvsEffectManager;

class LvsEffects {
public:
  struct effectProperties {
    LvsEasingFunctions::EaseType EASE{LvsEasingFunctions::LINEAR};

    uint32_t particle_amount{1};

    float delay{0.f};
    float duration{1.f};

    float particle_velocity{1.f};
    float particle_direction{0.f}; // in degrees

    float elapsed_delay_time{0.f};
    float elapsed_time{0.f};
    
    bool delay_finished{false};
    bool effected_by_gravity{true};

    LvsGameObject* particle{nullptr};
  };

  LvsEffects(LvsEffectManager& manager) : m_Manager{manager} {}

  int setEffect(effectProperties effect);

  void updateEffects(); 
  
  void deleteEffect();
  
  void pauseEffect();
  
  void continueEffect();
private:
  LvsEffectManager& m_Manager;
};

}