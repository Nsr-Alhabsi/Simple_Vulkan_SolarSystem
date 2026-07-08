#include "lvs_morph.hpp"
#include "lvs_SoA_morph.hpp"
#include "../../../ADDONS/cp_color.hpp"

// std:
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

namespace lvs {

// ============================================================
//  PUBLIC METHODS
// ============================================================


int LvsMorph::morphObject(VertexList  &verticesList, morphProperties &props) {
  if (checkMorphProperties(props, verticesList)) {
    int id = soa.free_slots.back();
    soa.free_slots.pop_back();
    soa.active_indices.push_back(id);

    syncSoAProperties(props, id);
    return id;
  }
  
  std::cerr << "LvsMorph::morphObject failed due to critical issues in morphProperties or verticesList. See above for details." << std::endl;
  return -1;
}

bool LvsMorph::checkMorphProperties(morphProperties &props, VertexList &verticesList) {
  std::vector<std::string> criticalIssues;
  std::vector<std::string> majorIssues;
  std::vector<std::string> minorIssues;

  // --- verticesList structural checks ---
  if (verticesList.empty()) {
    criticalIssues.push_back(
      "verticesList is empty, so there are no shapes to morph between. "
      "Fix: supply at least two shapes (std::vector<LvsModel::Vertex> entries) in verticesList before calling morphObject.");
  } else {
    // Every shape must be a non-zero multiple of 3 vertices: the pipeline uses
    // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST with no index buffer, so a malformed
    // vertex count reaches vkCmdDraw directly.
    std::string badShapes;
    for (size_t i = 0; i < verticesList.size(); ++i) {
      size_t count = verticesList[i].size();
      if (count == 0 || count % 3 != 0) {
        if (!badShapes.empty()) badShapes += ", ";
        badShapes += "index " + std::to_string(i) + " (" + std::to_string(count) + " vertices)";
      }
    }
    if (!badShapes.empty()) {
      criticalIssues.push_back(
        "The following shapes in verticesList have an invalid vertex count: " + badShapes + ". "
        "The pipeline uses VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST with no index buffer, so every shape's vertex count "
        "must be a non-zero multiple of 3. Fix: add or remove vertices so each shape's count is divisible by 3.");
    }

    int listSize = static_cast<int>(verticesList.size());
    int resolvedEnd = (props.end_index == -1) ? (listSize - 1) : props.end_index;

    bool startInRange = props.start_index >= 0 && props.start_index < listSize;
    bool endInRange    = resolvedEnd >= 0 && resolvedEnd < listSize;

    if (!startInRange) {
      criticalIssues.push_back(
        "props.start_index (" + std::to_string(props.start_index) + ") is out of range for verticesList "
        "(valid range is 0 to " + std::to_string(listSize - 1) + "). "
        "Fix: set start_index to a valid index within verticesList, or add more shapes to verticesList.");
    }
    if (!endInRange) {
      criticalIssues.push_back(
        "props.end_index resolves to " + std::to_string(resolvedEnd) + ", which is out of range for verticesList "
        "(valid range is 0 to " + std::to_string(listSize - 1) + "). "
        "Fix: set end_index to a valid index within verticesList, to -1 for the last element, or add more shapes to verticesList.");
    }

    if (startInRange && endInRange) {
      if (props.start_index == resolvedEnd) {
        majorIssues.push_back(
          "props.start_index and the resolved end_index both point to shape " + std::to_string(props.start_index) +
          ", so there is only one shape in the active sequence range and nothing to morph between. "
          "Fix: widen the range by changing start_index/end_index, or add more shapes to verticesList.");
      } else if (props.vertex_match_mode == MORPH_VERTEX_BY_INDEX) {
        int lo = std::min(props.start_index, resolvedEnd);
        int hi = std::max(props.start_index, resolvedEnd);
        size_t referenceCount = verticesList[lo].size();
        std::string mismatched;
        for (int i = lo + 1; i <= hi; ++i) {
          if (verticesList[i].size() != referenceCount) {
            if (!mismatched.empty()) mismatched += ", ";
            mismatched += "index " + std::to_string(i) + " (" + std::to_string(verticesList[i].size()) + " vertices)";
          }
        }
        if (!mismatched.empty()) {
          majorIssues.push_back(
            "vertex_match_mode is MORPH_VERTEX_BY_INDEX, which pairs source[i] with destination[i] directly, but "
            "shapes in the active range [" + std::to_string(lo) + ", " + std::to_string(hi) + "] have vertex counts "
            "that don't match shape " + std::to_string(lo) + " (" + std::to_string(referenceCount) + " vertices): " +
            mismatched + ". Fix: make every shape in the active range share the same vertex count, or switch "
            "vertex_match_mode to MORPH_VERTEX_BY_NEAREST or MORPH_VERTEX_COLLAPSE_EXPAND.");
        }
      }
    }
  }

  // --- Custom easing function pointers ---
  if (props.EASE == LvsEasingFunctions::CUSTOM_CURVE_STEER && props.MORPH_CUSTOM_EASE_FUNCTION == nullptr) {
    criticalIssues.push_back(
      "props.EASE is CUSTOM_CURVE_STEER but props.MORPH_CUSTOM_EASE_FUNCTION is nullptr, which would call a null "
      "function pointer every step. Fix: assign a valid float(*)(float) function to MORPH_CUSTOM_EASE_FUNCTION, "
      "or set EASE to one of the built-in curves.");
  }
  if (props.position_ease == LvsEasingFunctions::CUSTOM_CURVE_STEER && props.position_custom_ease_function == nullptr) {
    criticalIssues.push_back(
      "props.position_ease is CUSTOM_CURVE_STEER but props.position_custom_ease_function is nullptr, which would "
      "call a null function pointer every step. Fix: assign a valid float(*)(float) function to "
      "position_custom_ease_function, or set position_ease to one of the built-in curves.");
  }
  if (props.morph_vertex_colors && props.color_ease == LvsEasingFunctions::CUSTOM_CURVE_STEER &&
      props.color_custom_ease_function == nullptr) {
    criticalIssues.push_back(
      "props.morph_vertex_colors is enabled and props.color_ease is CUSTOM_CURVE_STEER but "
      "props.color_custom_ease_function is nullptr, which would call a null function pointer every step. "
      "Fix: assign a valid float(*)(float) function to color_custom_ease_function, set color_ease to one of the "
      "built-in curves, or disable morph_vertex_colors.");
  }
  if (props.morph_vertex_uvs && props.uv_ease == LvsEasingFunctions::CUSTOM_CURVE_STEER &&
      props.uv_custom_ease_function == nullptr) {
    criticalIssues.push_back(
      "props.morph_vertex_uvs is enabled and props.uv_ease is CUSTOM_CURVE_STEER but props.uv_custom_ease_function "
      "is nullptr, which would call a null function pointer every step. Fix: assign a valid float(*)(float) "
      "function to uv_custom_ease_function, set uv_ease to one of the built-in curves, or disable morph_vertex_uvs.");
  }

  // --- Repetition ---
  if (props.repetition == 0 || props.repetition < -1) {
    majorIssues.push_back(
      "props.repetition is " + std::to_string(props.repetition) + ", which is not a valid repetition count "
      "(0 means the morph would never play a single step; only -1 or a positive integer are valid). "
      "Fix: set repetition to -1 for infinite looping, or a positive integer for a fixed number of passes.");
  }

  // --- Core timing ---
  if (props.duration <= 0.0f) {
    majorIssues.push_back(
      "props.duration is " + std::to_string(props.duration) + ", which is not positive. Each step's progress is "
      "computed as elapsed_time / duration, so a non-positive duration produces division by zero, infinity, or NaN. "
      "Fix: set duration to a positive number of seconds.");
  }
  if (props.duration_variance < 0.0f) {
    majorIssues.push_back(
      "props.duration_variance is negative (" + std::to_string(props.duration_variance) + "). Variance fields are "
      "sampled uniformly from [0, variance] each time their timer resets, and a negative upper bound is undefined "
      "behaviour for that random sampling. Fix: set duration_variance to 0 or a positive number.");
  }
  if (props.delay_variance < 0.0f) {
    majorIssues.push_back(
      "props.delay_variance is negative (" + std::to_string(props.delay_variance) + "). Variance fields are "
      "sampled uniformly from [0, variance], and a negative upper bound is undefined behaviour for that random "
      "sampling. Fix: set delay_variance to 0 or a positive number.");
  }
  if (props.step_delay_variance < 0.0f) {
    majorIssues.push_back(
      "props.step_delay_variance is negative (" + std::to_string(props.step_delay_variance) + "). Variance fields "
      "are sampled uniformly from [0, variance], and a negative upper bound is undefined behaviour for that random "
      "sampling. Fix: set step_delay_variance to 0 or a positive number.");
  }

  if (props.delay < 0.0f) {
    minorIssues.push_back(
      "props.delay is negative (" + std::to_string(props.delay) + "), so the initial delay will effectively be "
      "treated as already elapsed and the morph will start immediately. Fix: set delay to 0 or a positive number "
      "of seconds if a startup delay is intended.");
  }
  if (props.step_delay < 0.0f) {
    minorIssues.push_back(
      "props.step_delay is negative (" + std::to_string(props.step_delay) + "), so the pause between steps will "
      "effectively be skipped. Fix: set step_delay to 0 or a positive number of seconds if a pause between steps "
      "is intended.");
  }
  if (props.loop_delay < 0.0f) {
    minorIssues.push_back(
      "props.loop_delay is negative (" + std::to_string(props.loop_delay) + "), so the pause between repetitions "
      "will effectively be skipped. Fix: set loop_delay to 0 or a positive number of seconds if a pause between "
      "repetitions is intended.");
  }
  if (props.hold_at_end_duration < 0.0f) {
    minorIssues.push_back(
      "props.hold_at_end_duration is negative (" + std::to_string(props.hold_at_end_duration) + "), so the hold on "
      "the final shape will effectively be skipped. Fix: set hold_at_end_duration to 0 or a positive number of "
      "seconds if a hold is intended.");
  }

  // --- Report ---
  auto printIssues = [](const std::vector<std::string> &issues, const std::string &color, const std::string &label) {
    for (size_t i = 0; i < issues.size(); ++i) {
      std::cerr << color << "[LvsMorph::checkMorphProperties] " << label << " (" << (i + 1) << "/" << issues.size()
                 << "): " << issues[i] << cpc::Reset << std::endl;
    }
  };

  if (!criticalIssues.empty()) {
    printIssues(criticalIssues, cpc::Red, "CRITICAL");
    if (!majorIssues.empty()) printIssues(majorIssues, cpc::Red, "MAJOR");
    if (!minorIssues.empty()) printIssues(minorIssues, cpc::Yellow, "MINOR");

    throw std::runtime_error(cpc::Red + "[LvsMorph::checkMorphProperties] " + std::to_string(criticalIssues.size()) +
      " critical issue(s) found in morphProperties/verticesList; see the CRITICAL messages above for details and "
      "fixes. Aborting before the morph could corrupt rendering state." + cpc::Reset);
  }

  if (!majorIssues.empty()) {
    printIssues(majorIssues, cpc::Red, "MAJOR");
    if (!minorIssues.empty()) printIssues(minorIssues, cpc::Yellow, "MINOR");
    std::cerr << cpc::Red << "[LvsMorph::checkMorphProperties] " << majorIssues.size()
               << " major issue(s) found — skipping this morph call until they are fixed (see messages above)."
               << cpc::Reset << std::endl;
    return false;
  }

  if (!minorIssues.empty()) {
    printIssues(minorIssues, cpc::Yellow, "MINOR");
    std::cerr << cpc::Yellow << "[LvsMorph::checkMorphProperties] " << minorIssues.size()
               << " minor issue(s) found — proceeding, but consider addressing them (see messages above)."
               << cpc::Reset << std::endl;
    return true;
  }

  std::cout << cpc::Green << "[LvsMorph::checkMorphProperties] morphProperties and verticesList passed validation "
                "with no issues found." << cpc::Reset << std::endl;
  return true;
}

void LvsMorph::syncSoAProperties(morphProperties &props, int idx, bool writeToSoA) {
  #define SYNC_VAL(field, vector) if(writeToSoA) vector[idx] = field; else field = static_cast<decltype(field)>(vector[idx]);

  // --- General ---
  SYNC_VAL(props.EASE,                       soa.morph_EASE);
  SYNC_VAL(props.MORPH_CUSTOM_EASE_FUNCTION, soa.morph_MORPH_CUSTOM_EASE_FUNCTION);
  // NOTE: MORPH_NAME is intentionally not synced — it's struct-only, never stored in the SoA.
  SYNC_VAL(props.active, soa.morph_active);

  // --- Core timing ---
  SYNC_VAL(props.duration,             soa.morph_duration);
  SYNC_VAL(props.duration_variance,    soa.morph_duration_variance);
  SYNC_VAL(props.delay,                soa.morph_delay);
  SYNC_VAL(props.delay_variance,       soa.morph_delay_variance);
  SYNC_VAL(props.step_delay,           soa.morph_step_delay);
  SYNC_VAL(props.step_delay_variance,  soa.morph_step_delay_variance);
  SYNC_VAL(props.hold_at_end_duration, soa.morph_hold_at_end_duration);

  // --- Sequencing ---
  SYNC_VAL(props.sequence_mode,             soa.morph_sequence_mode);
  SYNC_VAL(props.start_index,               soa.morph_start_index);
  SYNC_VAL(props.end_index,                 soa.morph_end_index);
  SYNC_VAL(props.snap_to_start_immediately, soa.morph_snap_to_start_immediately);

  // --- Repetition ---
  SYNC_VAL(props.repetition,        soa.morph_repetition);
  SYNC_VAL(props.reverse_on_finish, soa.morph_reverse_on_finish);
  SYNC_VAL(props.loop_delay,        soa.morph_loop_delay);
  SYNC_VAL(props.destroy_on_finish, soa.morph_destroy_on_finish);

  // --- Vertex interpolation control ---
  SYNC_VAL(props.position_ease,                 soa.morph_position_ease);
  SYNC_VAL(props.position_custom_ease_function, soa.morph_position_custom_ease_function);
  SYNC_VAL(props.morph_vertex_colors,           soa.morph_vertex_colors);
  SYNC_VAL(props.color_ease,                    soa.morph_color_ease);
  SYNC_VAL(props.color_custom_ease_function,    soa.morph_color_custom_ease_function);
  SYNC_VAL(props.morph_vertex_uvs,              soa.morph_vertex_uvs);
  SYNC_VAL(props.uv_ease,                       soa.morph_uv_ease);
  SYNC_VAL(props.uv_custom_ease_function,       soa.morph_uv_custom_ease_function);
  SYNC_VAL(props.vertex_match_mode,             soa.morph_vertex_match_mode);

  // --- Runtime / query fields ---
  SYNC_VAL(props.MORPH_STATE,           soa.morph_MORPH_STATE);
  SYNC_VAL(props.normalized_progress,   soa.morph_normalized_progress);
  SYNC_VAL(props.elapsed_time,          soa.morph_elapsed_time);
  SYNC_VAL(props.elapsed_delay_time,    soa.morph_elapsed_delay_time);
  SYNC_VAL(props.current_from_index,    soa.morph_current_from_index);
  SYNC_VAL(props.current_to_index,      soa.morph_current_to_index);
  SYNC_VAL(props.current_repetition,    soa.morph_current_repetition);
  SYNC_VAL(props.loop_delay_remaining,  soa.morph_loop_delay_remaining);
  SYNC_VAL(props.step_delay_remaining,  soa.morph_step_delay_remaining);
  SYNC_VAL(props.delay_finished,        soa.morph_delay_finished);
  SYNC_VAL(props.total_steps_completed, soa.morph_total_steps_completed);

  // --- Callbacks ---
  SYNC_VAL(props.callback_data,     soa.morph_callback_data);
  SYNC_VAL(props.on_morph_start,    soa.morph_on_morph_start);
  SYNC_VAL(props.on_step_complete,  soa.morph_on_step_complete);
  SYNC_VAL(props.on_morph_complete, soa.morph_on_morph_complete);

  // --- Randomness ---
  SYNC_VAL(props.random_seed, soa.morph_random_seed);

  #undef SYNC_VAL
}

}
