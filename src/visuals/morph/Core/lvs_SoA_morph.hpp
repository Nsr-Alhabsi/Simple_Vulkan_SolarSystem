#pragma once

// ============================================================
//  INCLUDES
// ============================================================

#include "lvs_morph.hpp"

// std:
#include <memory>
#include <vector>

namespace lvs {

// ============================================================
//  STRUCTS / TYPES
// ============================================================

/**
 * @brief Structure-of-Arrays storage for all per-morph-slot state.
 *
 * Every array is indexed by the morph's SoA slot index. Fields mirror the
 * LvsMorph::morphProperties struct exactly; a future sync routine (analogous
 * to the effects subsystem's syncPropertiesWithSoA) is expected to bridge
 * between the two representations.
 *
 * free_slots and active_indices form the pool bookkeeping: free_slots holds
 * indices available for new morph registrations, active_indices lists all
 * currently running morph slots iterated by the per-frame morph update pass.
 */
struct LvsSOAMorphs {

  // --- General ---
  std::unique_ptr<LvsEasingFunctions::EaseType[]> morph_EASE;
  std::unique_ptr<float(*[])(float)>               morph_MORPH_CUSTOM_EASE_FUNCTION;
  // NOTE: MORPH_NAME (std::string) is intentionally omitted — it exists only on
  // morphProperties for debugging and is never stored in the SoA.
  std::unique_ptr<bool[]> morph_active;

  // --- Core timing ---
  std::unique_ptr<float[]> morph_duration;
  std::unique_ptr<float[]> morph_duration_variance;
  std::unique_ptr<float[]> morph_delay;
  std::unique_ptr<float[]> morph_delay_variance;
  std::unique_ptr<float[]> morph_step_delay;
  std::unique_ptr<float[]> morph_step_delay_variance;
  std::unique_ptr<float[]> morph_hold_at_end_duration;

  // --- Sequencing ---
  std::unique_ptr<MorphSequenceMode[]> morph_sequence_mode;
  std::unique_ptr<int[]>               morph_start_index;
  std::unique_ptr<int[]>               morph_end_index;
  std::unique_ptr<bool[]>              morph_snap_to_start_immediately;

  // --- Repetition ---
  std::unique_ptr<int[]>   morph_repetition;
  std::unique_ptr<bool[]>  morph_reverse_on_finish;
  std::unique_ptr<float[]> morph_loop_delay;
  std::unique_ptr<bool[]>  morph_destroy_on_finish;

  // --- Vertex interpolation control ---
  std::unique_ptr<LvsEasingFunctions::EaseType[]> morph_position_ease;
  std::unique_ptr<float(*[])(float)>               morph_position_custom_ease_function;
  std::unique_ptr<bool[]>                          morph_vertex_colors;
  std::unique_ptr<LvsEasingFunctions::EaseType[]>  morph_color_ease;
  std::unique_ptr<float(*[])(float)>               morph_color_custom_ease_function;
  std::unique_ptr<bool[]>                          morph_vertex_uvs;
  std::unique_ptr<LvsEasingFunctions::EaseType[]>  morph_uv_ease;
  std::unique_ptr<float(*[])(float)>               morph_uv_custom_ease_function;
  std::unique_ptr<MorphVertexMatchMode[]>          morph_vertex_match_mode;

  // --- Runtime / query fields ---
  std::unique_ptr<MorphState[]> morph_MORPH_STATE;
  std::unique_ptr<float[]>      morph_normalized_progress;
  std::unique_ptr<float[]>      morph_elapsed_time;
  std::unique_ptr<float[]>      morph_elapsed_delay_time;
  std::unique_ptr<int[]>        morph_current_from_index;
  std::unique_ptr<int[]>        morph_current_to_index;
  std::unique_ptr<int[]>        morph_current_repetition;
  std::unique_ptr<float[]>      morph_loop_delay_remaining;
  std::unique_ptr<float[]>      morph_step_delay_remaining;
  std::unique_ptr<bool[]>       morph_delay_finished;
  std::unique_ptr<int[]>        morph_total_steps_completed;

  // --- Callbacks ---
  std::unique_ptr<void*[]>               morph_callback_data;
  std::unique_ptr<void(*[])(void*)>      morph_on_morph_start;
  std::unique_ptr<void(*[])(int, void*)> morph_on_step_complete;
  std::unique_ptr<void(*[])(void*)>      morph_on_morph_complete;

  // --- Randomness ---
  std::unique_ptr<uint32_t[]> morph_random_seed;

  // --- Pool bookkeeping ---
  /** @brief Indices of morph slots currently running; iterated by the per-frame morph update pass. */
  std::vector<int> active_indices;

  /** @brief Indices of morph slots available for new morph registrations. */
  std::vector<int> free_slots;
};

}
