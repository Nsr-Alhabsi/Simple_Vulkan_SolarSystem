#pragma once

#include "lvs_effect_manager.hpp"
#include "../../utils/easing_functions.hpp"
#include "../../models/lvs_game_object.hpp"
namespace lvs {
class LvsEffects {
public:
  struct effectProperties {
    LvsEasingFunctions::EaseType EASE{LvsEasingFunctions::LINEAR};

    uint32_t particle_amount{1};

    float delay{0.f};
    float duration{1.f};

    float elapsed_delay_time{0.f};
    float elapsed_time{0.f};
    
    bool delay_finished{false};

    LvsGameObject* particle{nullptr};
  };

  int setEffect(effectProperties effect) { return m_Manager.initializeEffect(effect); };

  void updateEffects(); 
  
  void deleteEffect();
  
  void pauseEffect();
  
  void continueEffect();
private:
  LvsEffectManager& m_Manager;
};

}