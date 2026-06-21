#pragma once

#include "../models/lvs_game_object.hpp"
namespace lvs {
class LvsParticleSystem {

public:
  void updateParticlePosition(LvsGameObject &particle);

  LvsParticleSystem() = default;
private:
  void updateParticleRotation(LvsGameObject &particle);
  void updateParticleScale(LvsGameObject &particle);
  void updateParticleTranslation(LvsGameObject &particle);

};
}