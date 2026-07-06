#include "lvs_effects.hpp"
#include "../lvs_effect_manager.hpp"

// ============================================================
//  PUBLIC METHODS
// ============================================================

namespace lvs {

int LvsEffects::setEffect(effectProperties effect) {
  m_EffectIdx = m_Manager.initializeEffect(effect);
  return m_EffectIdx;
}

void LvsEffects::pauseEffect(int idx) {
  m_Manager.pauseEffect(idx);
}

void LvsEffects::continueEffect(int idx) {
  m_Manager.continueEffect(idx);
}

void LvsEffects::deleteEffect(int idx) {
  m_Manager.deleteEffect(idx);
  if (idx == m_EffectIdx) m_EffectIdx = -1;
}

} // namespace lvs
