#pragma once

namespace lvs {

// ============================================================
//  ENUMS
// ============================================================

/**
 * @brief Controls which direction the morph sequence advances through `verticesList`.
 */
enum MorphSequenceMode {
  MORPH_SEQUENCE_FORWARD,    ///< @brief Advance 0 -> 1 -> 2 -> ... -> N.
  MORPH_SEQUENCE_REVERSE,    ///< @brief Advance N -> N-1 -> ... -> 0.
  MORPH_SEQUENCE_PING_PONG,  ///< @brief Alternate 0->...->N->N-1->...->0 each repetition.
  MORPH_SEQUENCE_RANDOM      ///< @brief Jump to a random next shape each step (no immediate repeat).
};

/**
 * @brief Controls how source and destination vertices are paired during interpolation.
 */
enum MorphVertexMatchMode {
  MORPH_VERTEX_BY_INDEX,        ///< @brief Pair source[i] with destination[i] directly.
  MORPH_VERTEX_BY_NEAREST,      ///< @brief Match each destination vertex to the nearest source vertex by position.
  MORPH_VERTEX_COLLAPSE_EXPAND  ///< @brief Replicate or collapse vertices when source and destination counts differ.
};

/**
 * @brief Runtime lifecycle state of a morph slot — written by the system, read by the user.
 */
enum MorphState {
  MORPH_STATE_STARTING,    ///< @brief Morph has been registered but has not yet processed its first tick.
  MORPH_STATE_DELAYING,    ///< @brief Waiting out the initial `delay` before the first step begins.
  MORPH_STATE_PLAYING,     ///< @brief Actively interpolating between two shapes.
  MORPH_STATE_STEP_DELAY,  ///< @brief Holding between consecutive steps (`step_delay`).
  MORPH_STATE_LOOP_DELAY,  ///< @brief Holding between repetition cycles (`loop_delay`).
  MORPH_STATE_FINISHED,    ///< @brief All repetitions complete; morph is no longer updating.
  MORPH_STATE_PAUSED,      ///< @brief Update loop is suspended; mesh stays at its current shape.
  MORPH_STATE_DELETED      ///< @brief Slot has been freed; accessing it is undefined behaviour.
};

}  // namespace lvs
