#pragma once

// ============================================================
//  INCLUDES
// ============================================================

#include "core/lvs_effects.hpp"
#include "core/lvs_SoA_effects.hpp"
#include "lvs_particle_system.hpp"

#include "../../core/lvs_device.hpp"

#include <random>

namespace lvs {

// ============================================================
//  CLASS DECLARATION
// ============================================================

/**
 * @brief Owns and drives the full lifecycle of all particle effects.
 *
 * LvsEffectManager allocates the SoA arrays for both effects and particles
 * in a single up-front call to init(), then vends effect slots through
 * initializeEffect(). Each frame, updateEffects() (or updateEffect() for a
 * single slot) advances spawn timers, drives per-particle physics via
 * LvsParticleSystem, and recycles dead particles back into the per-effect
 * free list.
 *
 * Particle memory is managed with a bump allocator (m_ParticlePoolCursor)
 * that carves out a contiguous sub-range of the flat particle pool for each
 * new effect. Per-effect free lists (m_ParticleFreeSlots) track which local
 * slots within that sub-range are available for the next spawn.
 */
class LvsEffectManager {
public:

  // ------------------------------------------------------------
  //  CONSTRUCTORS / DESTRUCTOR
  // ------------------------------------------------------------

  /**
   * @brief Constructs the manager, binding it to a Vulkan device.
   * @param device Vulkan device used for any GPU-side operations.
   */
  LvsEffectManager(LvsDevice& device)
      : lvsDevice{device}, m_Rng{std::random_device{}()}, particleSystem{soa, particleSoa, m_Rng} {}

  // ------------------------------------------------------------
  //  PUBLIC API
  // ------------------------------------------------------------

  /**
   * @brief Allocates all SoA arrays and initializes pool bookkeeping.
   *
   * Must be called once before any other method. Calling it a second time
   * would leak the previously allocated arrays.
   *
   * @param count               Maximum number of concurrent effects.
   * @param max_particles_total Maximum number of particles that can be alive across all effects.
   */
  void init(uint32_t count, uint32_t max_particles_total);

  /**
   * @brief Validates, clamps, and registers a new effect into the pool.
   *
   * Derives the per-effect particle cap via calculateMaxPresistentParticles(),
   * carves out a slice of the flat particle pool with the bump allocator, and
   * writes all properties into the SoA via syncPropertiesWithSoA().
   *
   * @param effect User-populated effectProperties describing the desired effect.
   * @return The SoA slot index assigned to this effect; pass to pause/continue/delete.
   */
  int initializeEffect(LvsEffects::effectProperties effect);

  /**
   * @brief Advances a single effect by one timestep.
   *
   * Handles loop-delay countdown, burst or continuous spawning, per-particle
   * physics updates, lifetime expiry, and the on_effect_finish callback.
   *
   * @param effect_idx SoA slot index of the effect to update.
   * @param dt         Delta time in seconds since the last frame.
   */
  void updateEffect(int effect_idx, float dt);

  /**
   * @brief Advances all active effects by one timestep.
   * @param dt Delta time in seconds since the last frame.
   */
  void updateEffects(float dt);

  /**
   * @brief Suspends an effect so it stops spawning and updating.
   * @param idx SoA slot index of the effect to pause.
   */
  void pauseEffect(int idx);

  /**
   * @brief Resumes a previously paused effect.
   * @param idx SoA slot index of the effect to resume.
   */
  void continueEffect(int idx);

  /**
   * @brief Kills all live particles belonging to an effect and returns its slot to the pool.
   * @param idx SoA slot index of the effect to delete.
   */
  void deleteEffect(int idx);

  /**
   * @brief Returns a read-only view of the effect SoA arrays.
   * @return Const reference to LvsSOAEffects.
   */
  const LvsSOAEffects&   getEffectSoA()   const { return soa; }

  /**
   * @brief Returns a read-only view of the particle SoA arrays.
   * @return Const reference to LvsSOAParticles.
   */
  const LvsSOAParticles& getParticleSoA() const { return particleSoa; }

  /**
   * @brief Reads every property of a live effect back out of the SoA.
   *
   * @param idx SoA slot index of the effect.
   * @return    An effectProperties struct reconstructed from the SoA.
   * @throws std::runtime_error if idx is out of range for the allocated pool.
   */
  LvsEffects::effectProperties getEffectProperties(int idx);

  /**
   * @brief Reads a single field from a live effect's SoA state.
   *
   * Reconstructs a temporary effectProperties from the SoA and returns
   * the requested member. Explicit instantiations cover float, int, bool,
   * glm::vec2, glm::vec3, uint32_t, EaseType, and LvsGameObject*.
   *
   * @tparam T         Type of the field to retrieve.
   * @param  idx       SoA slot index of the effect.
   * @param  field     Pointer-to-member selecting which effectProperties field to read.
   * @return           Current value of that field in the SoA.
   * @throws std::runtime_error if idx is out of range for the allocated pool.
   */
  template<typename T>
  T getEffectProperties(int idx, T LvsEffects::effectProperties::* field);

private:

  // ------------------------------------------------------------
  //  PRIVATE MEMBERS
  // ------------------------------------------------------------

  uint32_t m_MaxEffects;
  uint32_t m_MaxParticles;
  uint32_t m_ParticlePoolCursor;

  LvsSOAEffects   soa;
  LvsSOAParticles particleSoa;

  // Per-effect free list of available local particle slots within the
  // sub-range carved out for that effect.
  std::vector<std::vector<int>> m_ParticleFreeSlots;

  std::mt19937     m_Rng;
  LvsDevice&       lvsDevice;
  LvsParticleSystem particleSystem;

  // ------------------------------------------------------------
  //  PRIVATE HELPERS
  // ------------------------------------------------------------

  // Derives the maximum number of concurrently alive particles for an
  // effect based on its spawn rate, duration, burst settings, and mode.
  int calculateMaxPresistentParticles(LvsEffects::effectProperties &props);

  // Pops the next available local slot from the effect's free list and
  // delegates to LvsParticleSystem::initalizeParticle.
  void spawnParticle(int effect_idx);

  // Bridges an effectProperties struct and the parallel SoA arrays.
  // writeToSOA == true copies struct → SoA; false copies SoA → struct.
  void syncPropertiesWithSoA(int idx, LvsEffects::effectProperties &props, bool writeToSOA);

  // Throws if idx is outside the allocated pool; warns (non-fatal) if idx is
  // in range but not a currently active effect slot.
  void validateEffectIndex(int idx, const char* callerName) const;
};

} // namespace lvs
