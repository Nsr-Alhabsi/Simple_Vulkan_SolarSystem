#include "lvs_effects.hpp"
#include "../lvs_effect_manager.hpp"

namespace lvs {

int LvsEffects::setEffect(effectProperties effect) {
    return m_Manager.initializeEffect(effect);
}

void LvsEffects::updateEffects() {
    // TODO: implement
}

void LvsEffects::deleteEffect() {
    // TODO: implement
}

void LvsEffects::pauseEffect() {
    // TODO: implement
}

void LvsEffects::continueEffect() {
    // TODO: implement
}

}