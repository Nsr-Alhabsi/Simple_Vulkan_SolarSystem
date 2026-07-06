#pragma once

// ============================================================
//  INCLUDES
// ============================================================

#include "Core/lvs_SoA_effects.hpp"

#include <random>

namespace lvs {

// ============================================================
//  CLASS DECLARATION
// ============================================================

/**
 * @brief Handles per-particle initialization and per-frame physics updates.
 *
 * LvsParticleSystem operates entirely on the shared SoA arrays owned by
 * LvsEffectManager — it holds only references to those arrays and to the
 * shared RNG. All particle logic (spawn, translation, rotation, scale) is
 * expressed as index-based operations on the flat SoA arrays so that the
 * hot update path stays cache-friendly.
 */
class LvsParticleSystem {
public:

  // ------------------------------------------------------------
  //  CONSTRUCTORS / DESTRUCTOR
  // ------------------------------------------------------------

  /**
   * @brief Constructs the particle system, binding it to the shared SoA arrays and RNG.
   * @param effectSoa   SoA arrays for effect-level state (spawn settings, easing, etc.).
   * @param particleSoa SoA arrays for per-particle state (position, velocity, color, etc.).
   * @param rng         Shared Mersenne Twister used for all randomness during spawn.
   */
  LvsParticleSystem(LvsSOAEffects& effectSoa, LvsSOAParticles& particleSoa, std::mt19937& rng)
      : m_EffectSoa{effectSoa}, m_ParticleSoa{particleSoa}, m_Rng{rng} {}

  // ------------------------------------------------------------
  //  PUBLIC API
  // ------------------------------------------------------------

  /**
   * @brief Initializes a particle slot with randomized spawn-time values.
   *
   * Reads effect properties from the SoA, applies variance to position,
   * direction, velocity, angular velocity, scale, color, and opacity, then
   * snapshots the resulting values as stable interpolation baselines. Fires
   * the on_particle_spawn callback if set.
   *
   * @param effect_idx SoA slot index of the owning effect.
   * @param local_slot Local slot index within that effect's particle sub-range.
   */
  void initalizeParticle(int effect_idx, int local_slot);

  /**
   * @brief Advances a single particle's age, color, opacity, and physics for one timestep.
   *
   * Increments p_age, interpolates color and opacity (with fade-in/fade-out),
   * then delegates to updateParticleTranslation, updateParticleRotation, and
   * updateParticleScale.
   *
   * @param abs_idx Absolute index into the flat particle SoA arrays.
   * @param dt      Delta time in seconds since the last frame.
   */
  void updateParticlePosition(int abs_idx, float dt);

private:

  // ------------------------------------------------------------
  //  PRIVATE HELPERS
  // ------------------------------------------------------------

  // Interpolates angular velocity from spawn value to end value and applies
  // it to p_direction, effectively steering the particle's travel direction.
  void updateParticleRotation(int abs_idx, float dt);

  // Interpolates p_scale from the spawn-time snapshot toward effect_particle_scale_end
  // using the effect's scale easing curve.
  void updateParticleScale(int abs_idx, float dt);

  // Interpolates speed from p_velocity_start toward effect_particle_velocity_end,
  // applies drag, then integrates position along p_direction plus acceleration and gravity.
  void updateParticleTranslation(int abs_idx, float dt);

  // ------------------------------------------------------------
  //  PRIVATE MEMBERS
  // ------------------------------------------------------------

  LvsSOAEffects&   m_EffectSoa;
  LvsSOAParticles& m_ParticleSoa;
  std::mt19937&    m_Rng;
};

} // namespace lvs
