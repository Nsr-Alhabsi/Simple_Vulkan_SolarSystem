#pragma once

// ============================================================
//  INCLUDES
// ============================================================

#include "../../../models/lvs_model.hpp"
#include "../../../utils/easing_functions.hpp"

#include <string>
#include <vector>

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

// ============================================================
//  CLASS DECLARATION
// ============================================================

/**
 * @brief Vertex morph system that interpolates a mesh through a sequence of shapes over time.
 *
 * `LvsMorph` drives per-vertex animation by blending between entries in a `verticesList` —
 * a caller-supplied list of vertex arrays, each defining one target shape. The system
 * advances through shapes according to `morphProperties`, applying easing, timing, repetition,
 * and optional per-vertex color/UV interpolation.
 */
class LvsMorph {
public:

  // ------------------------------------------------------------
  //  PUBLIC API
  // ------------------------------------------------------------

  /**
   * @brief Configuration struct passed to `morphObject` to define all morph behaviour.
   *
   * All timing fields are in seconds. Variance fields add a random offset uniformly
   * sampled from `[0, variance]` each time the corresponding timer is reset.
   * Runtime / query fields are written by the system each frame — do not set them manually.
   */
  struct morphProperties {

    // --- General ---

    /// @brief Easing curve applied to the overall morph progress (`t`) each step.
    LvsEasingFunctions::EaseType EASE = LvsEasingFunctions::LINEAR;

    /// @brief Custom easing function pointer used when `EASE == CUSTOM_CURVE_STEER`.
    float (*MORPH_CUSTOM_EASE_FUNCTION)(float) = nullptr;

    /// @brief Human-readable label for debugging.
    /// @note This string is not stored in the SoA — it exists only on this struct.
    std::string MORPH_NAME = "";

    /// @brief When `false` the system skips this morph entirely each frame without freeing its slot.
    bool active = true;

    // --- Core timing ---

    /// @brief Duration of each individual shape-to-shape step, in seconds.
    float duration = 1.0f;

    /// @brief Maximum random extra seconds added to `duration` per step.
    float duration_variance = 0.0f;

    /// @brief Seconds to wait before the very first step begins.
    float delay = 0.0f;

    /// @brief Maximum random extra seconds added to `delay`.
    float delay_variance = 0.0f;

    /// @brief Pause duration inserted between consecutive steps.
    float step_delay = 0.0f;

    /// @brief Maximum random extra seconds added to `step_delay`.
    float step_delay_variance = 0.0f;

    /// @brief Seconds to hold on the final shape before looping or finishing.
    float hold_at_end_duration = 0.0f;

    // --- Sequencing ---

    /// @brief Order in which shapes are visited across steps.
    MorphSequenceMode sequence_mode = MORPH_SEQUENCE_FORWARD;

    /// @brief Index into `verticesList` at which the sequence begins.
    int start_index = 0;

    /// @brief Index into `verticesList` at which the sequence ends; `-1` means the last element.
    int end_index = -1;

    /// @brief When `true`, the mesh is snapped to the first shape on the very first frame without interpolating.
    bool snap_to_start_immediately = true;

    // --- Repetition ---

    /// @brief Number of full sequence passes to perform; `-1` means infinite.
    int repetition = 1;

    /// @brief When `true`, the sequence direction reverses after each completed pass instead of restarting.
    bool reverse_on_finish = false;

    /// @brief Seconds to wait between the end of one repetition and the start of the next.
    float loop_delay = 0.0f;

    /// @brief When `true`, the morph slot is freed after all repetitions complete.
    /// @note Defaults to `false` — unlike the effects system — so the mesh remains at its
    ///       final interpolated shape after the morph finishes without requiring re-submission.
    bool destroy_on_finish = false;

    // --- Vertex interpolation control ---

    /// @brief Easing curve applied specifically to vertex positions, independent of `EASE`.
    /// @note When both `position_ease` and `EASE` are `LINEAR` the result is identical.
    ///       Set `position_ease` to a different curve to ease positions separately from any
    ///       other per-attribute easing (e.g., color or UV).
    LvsEasingFunctions::EaseType position_ease = LvsEasingFunctions::LINEAR;

    /// @brief Custom position easing function pointer used when `position_ease == CUSTOM_CURVE_STEER`.
    float (*position_custom_ease_function)(float) = nullptr;

    /// @brief When `true`, per-vertex colors are interpolated alongside positions.
    /// @note Vertex colors are stored and blended correctly, but have no visual effect
    ///       until the shader gains per-vertex color support.
    bool morph_vertex_colors = false;

    /// @brief Easing curve applied to per-vertex color interpolation.
    LvsEasingFunctions::EaseType color_ease = LvsEasingFunctions::LINEAR;

    /// @brief Custom color easing function pointer used when `color_ease == CUSTOM_CURVE_STEER`.
    float (*color_custom_ease_function)(float) = nullptr;

    /// @brief When `true`, per-vertex UV coordinates are interpolated alongside positions.
    /// @note Per-vertex UVs are stored and blended correctly, but have no visual effect
    ///       until the shader gains per-vertex UV support.
    bool morph_vertex_uvs = false;

    /// @brief Easing curve applied to per-vertex UV interpolation.
    LvsEasingFunctions::EaseType uv_ease = LvsEasingFunctions::LINEAR;

    /// @brief Custom UV easing function pointer used when `uv_ease == CUSTOM_CURVE_STEER`.
    float (*uv_custom_ease_function)(float) = nullptr;

    /// @brief Strategy used to pair source and destination vertices when counts may differ.
    MorphVertexMatchMode vertex_match_mode = MORPH_VERTEX_BY_INDEX;

    // --- Runtime / query fields ---

    /// @brief Current lifecycle state of this morph slot.
    /// @note Set by the system at runtime. Do not write to this field manually.
    MorphState MORPH_STATE = MORPH_STATE_STARTING;

    /// @brief Eased progress in [0, 1] for the current step, after applying `EASE`.
    /// @note Set by the system at runtime. Do not write to this field manually.
    float normalized_progress = 0.0f;

    /// @brief Raw seconds elapsed within the current active step (unaffected by easing).
    /// @note Set by the system at runtime. Do not write to this field manually.
    float elapsed_time = 0.0f;

    /// @brief Raw seconds elapsed within the current delay phase.
    /// @note Set by the system at runtime. Do not write to this field manually.
    float elapsed_delay_time = 0.0f;

    /// @brief `verticesList` index of the shape currently being interpolated from.
    /// @note Set by the system at runtime. Do not write to this field manually.
    int current_from_index = 0;

    /// @brief `verticesList` index of the shape currently being interpolated toward.
    /// @note Set by the system at runtime. Do not write to this field manually.
    int current_to_index = 1;

    /// @brief Number of full sequence passes completed so far.
    /// @note Set by the system at runtime. Do not write to this field manually.
    int current_repetition = 0;

    /// @brief Seconds remaining in the current loop delay phase.
    /// @note Set by the system at runtime. Do not write to this field manually.
    float loop_delay_remaining = 0.0f;

    /// @brief Seconds remaining in the current step delay phase.
    /// @note Set by the system at runtime. Do not write to this field manually.
    float step_delay_remaining = 0.0f;

    /// @brief `true` once the initial delay (including variance) has elapsed.
    /// @note Set by the system at runtime. Do not write to this field manually.
    bool delay_finished = false;

    /// @brief Cumulative count of individual shape-to-shape steps completed across all repetitions.
    /// @note Set by the system at runtime. Do not write to this field manually.
    int total_steps_completed = 0;

    // --- Callbacks ---

    /// @brief Opaque pointer forwarded to every callback as its last argument.
    void *callback_data = nullptr;

    /// @brief Called once when the morph transitions out of `MORPH_STATE_STARTING`.
    void (*on_morph_start)(void *) = nullptr;

    /// @brief Called each time a step completes; the `int` argument is the index just reached in `verticesList`.
    /// @note The shape index is passed directly rather than through `callback_data` because it
    ///       cannot reasonably be routed via a generic `void*` without an extra allocation.
    void (*on_step_complete)(int, void *) = nullptr;

    /// @brief Called once when all repetitions are complete and the morph enters `MORPH_STATE_FINISHED`.
    void (*on_morph_complete)(void *) = nullptr;

    // --- Randomness ---

    /// @brief Seed for the internal RNG used by variance fields and `MORPH_SEQUENCE_RANDOM`.
    /// @note `0` lets the system choose a seed automatically.
    uint32_t random_seed = 0;
  };

  /**
   * @brief Registers and drives a per-frame vertex morph on the supplied vertex list.
   *
   * Each call advances the morph by one frame tick according to `props`. The system
   * writes all runtime / query fields on `props` before returning, so callers can
   * inspect `MORPH_STATE` and `normalized_progress` immediately after the call.
   *
   * @param verticesList Ordered list of vertex arrays; each entry is one target shape.
   *                     The morph sequences through these according to `props.sequence_mode`.
   * @param props        Morph configuration and runtime state. Mutated in-place each frame.
   */
  void morphObject(std::vector<std::vector<LvsModel::Vertex>> &verticesList, morphProperties &props);

private:

};

}  // namespace lvs
