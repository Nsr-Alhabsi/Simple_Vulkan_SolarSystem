#pragma once

#include "Core/lvs_morph.hpp"
#include "../../models/lvs_model.hpp"

#include <cstdint>
#include <vector>

namespace lvs {

/**
 * @brief Per-tick driver and interpolation math for a single LvsMorph slot.
 *
 * `LvsMorph` (Core/lvs_morph.hpp) owns registration, validation, and SoA storage for morph
 * slots but does not itself advance them frame to frame. `LvsMorphCalculation::calculateMorph`
 * is that missing per-tick step: given the shapes to morph between and a slot's current
 * `morphProperties`, it advances the slot's state machine by `deltaTime`, computes the
 * interpolated vertex array for whatever step is now active, and writes every runtime/query
 * field on `props` to reflect the result — mirroring the "system writes runtime fields"
 * contract already documented on `morphProperties` for `LvsMorph::morphObject`.
 *
 * This class does not touch the SoA or pool bookkeeping (`free_slots`/`active_indices`) —
 * a future per-frame driver is expected to read a slot's `morphProperties` out of the SoA,
 * call `calculateMorph`, apply the returned vertices via `LvsGameObject::setVertices`, then
 * sync `props` back into the SoA (analogous to `LvsMorph::syncSoAProperties`).
 */
class LvsMorphCalculation {
public:
  /**
   * @brief Advances one morph slot by `deltaTime` and returns the resulting interpolated mesh.
   *
   * Drives `props.MORPH_STATE` through STARTING -> (DELAYING) -> PLAYING -> (STEP_DELAY) ->
   * ... -> (LOOP_DELAY, reused for both the between-repetition pause and the final
   * `hold_at_end_duration` hold — see the .cpp for why a single state can serve both) ->
   * FINISHED. `active == false`, `MORPH_STATE_PAUSED`, and `MORPH_STATE_DELETED` all freeze
   * the slot: no time is consumed and `current_vertices` is returned unchanged.
   *
   * @param verticesList Ordered list of target shapes (see LvsMorph::morphObject). Every shape
   *                     is assumed to already satisfy the multiple-of-3 vertex count validated
   *                     by LvsMorph::checkMorphProperties.
   * @param props        Mutated in place: every runtime/query field described on
   *                     LvsMorph::morphProperties as "set by the system at runtime" is updated
   *                     to reflect the state after this tick.
   * @param deltaTime    Seconds elapsed since the previous call for this morph slot. May be 0
   *                     (state still initializes/settles) or span multiple completed
   *                     steps/repetitions in one call; both are handled correctly.
   * @return             The interpolated vertex array now current for this morph — identical
   *                     to `props.current_vertices` after the call.
   */
  std::vector<LvsModel::Vertex> calculateMorph(
    const std::vector<std::vector<LvsModel::Vertex>> &verticesList, LvsMorph::morphProperties &props, float deltaTime
  );

private:
  // ------------------------------------------------------------
  //  EASING
  // ------------------------------------------------------------

  // Dispatches EaseType -> the matching LvsEasingFunctions::*_FUNCTION, mirroring the
  // established pattern in src/visuals/effects/lvs_particle_system.cpp (applyEase). Duplicated
  // locally rather than shared, consistent with each visuals subsystem owning its own copy.
  static float applyEase(LvsEasingFunctions::EaseType ease, float (*customFn)(float), float t);

  // ------------------------------------------------------------
  //  DETERMINISTIC RANDOMNESS
  // ------------------------------------------------------------

  // Every random draw this class makes (variance sampling, MORPH_SEQUENCE_RANDOM index choice)
  // is a pure function of (effective seed, a domain tag, a step id already stored on props) —
  // no RNG engine state is persisted anywhere, keeping calculateMorph a function of its
  // arguments only. See the .cpp for the effective-seed derivation when random_seed == 0.
  static uint32_t hashCombine(uint32_t seed, uint32_t domain, uint32_t stepId);
  static float sampleVariance(uint32_t seed, uint32_t domain, uint32_t stepId, float variance);
  static int pickRandomIndex(int lo, int hi, int excluding, uint32_t seed, uint32_t stepId);

  // ------------------------------------------------------------
  //  SEQUENCING
  // ------------------------------------------------------------

  static int resolveEndIndex(const LvsMorph::morphProperties &props, int listSize);

  // Advances current_from_index/current_to_index by exactly one step for the active
  // sequence_mode. Returns true if that step exhausted the current pass (the caller then
  // handles repetition bookkeeping); returns false if there is another step left in this pass.
  static bool advanceWithinPass(LvsMorph::morphProperties &props, int resolvedEnd, uint32_t effectiveSeed);

  // Resets current_from_index/current_to_index for the start of a fresh pass, honoring
  // reverse_on_finish for FORWARD/REVERSE (ignored for RANDOM, which has no notion of
  // direction — see the .cpp).
  static void resetIndicesForNextPass(LvsMorph::morphProperties &props, int resolvedEnd, uint32_t effectiveSeed);

  // ------------------------------------------------------------
  //  VERTEX INTERPOLATION
  // ------------------------------------------------------------

  static std::vector<LvsModel::Vertex> interpolateShapes(
    const std::vector<LvsModel::Vertex> &from, const std::vector<LvsModel::Vertex> &to,
    const LvsMorph::morphProperties &props, float posT, float colorT, float uvT
  );

  // MORPH_VERTEX_BY_NEAREST: builds a from-array the same length as `to`, pairing each
  // destination vertex with its nearest source vertex by position.
  static std::vector<LvsModel::Vertex> matchByNearest(
    const std::vector<LvsModel::Vertex> &from, const std::vector<LvsModel::Vertex> &to
  );

  // MORPH_VERTEX_COLLAPSE_EXPAND: replicates `shape`'s last vertex to expand it, or collapses
  // every vertex beyond targetCount-1 onto a single final slot to shrink it, so it reaches
  // exactly targetCount entries for direct index-wise pairing.
  static std::vector<LvsModel::Vertex> padToCount(const std::vector<LvsModel::Vertex> &shape, size_t targetCount);
};

}
