#pragma once

namespace lvs {
class LvsParticleSystem {

public:
  void updateParticlePosition(int idx);

  LvsParticleSystem() = default;
private:
  void updateParticleRotation(int idx);
  void updateParticleScale(int idx);
  void updateParticleTranslation(int idx);

};

}