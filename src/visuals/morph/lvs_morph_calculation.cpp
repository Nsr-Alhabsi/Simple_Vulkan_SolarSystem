#include "lvs_morph_calculation.hpp"
#include "../../ADDONS/cp_color.hpp"

// std:
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>

namespace lvs {

// ============================================================
//  CONSTANTS
// ============================================================

// Domain tags keep independently-sampled quantities (which all derive from the same
// (seed, stepId) pair) from producing correlated values.
static constexpr uint32_t DOMAIN_DELAY        = 1;
static constexpr uint32_t DOMAIN_DURATION     = 2;
static constexpr uint32_t DOMAIN_STEP_DELAY   = 3;
static constexpr uint32_t DOMAIN_RANDOM_INDEX = 4;

// Safety cap on how many state-machine transitions a single calculateMorph call may process.
// Only matters for pathological deltaTime spikes (e.g. a debugger pause); every real morph
// configuration terminates in far fewer iterations since durations are validated > 0.
static constexpr int MAX_TICK_ITERATIONS = 100000;

// ============================================================
//  EASING
// ============================================================

float LvsMorphCalculation::applyEase(LvsEasingFunctions::EaseType ease, float (*customFn)(float), float t) {
  using E = LvsEasingFunctions;
  if (ease == E::CUSTOM_CURVE_STEER && customFn) return customFn(t);
  static float (*const TABLE[])(float) = {
    E::LINEAR_FUNCTION,
    E::EASE_IN_SINE_FUNCTION,     E::EASE_OUT_SINE_FUNCTION,     E::EASE_IN_OUT_SINE_FUNCTION,
    E::EASE_IN_QUAD_FUNCTION,     E::EASE_OUT_QUAD_FUNCTION,     E::EASE_IN_OUT_QUAD_FUNCTION,
    E::EASE_IN_CUBIC_FUNCTION,    E::EASE_OUT_CUBIC_FUNCTION,    E::EASE_IN_OUT_CUBIC_FUNCTION,
    E::EASE_IN_QUART_FUNCTION,    E::EASE_OUT_QUART_FUNCTION,    E::EASE_IN_OUT_QUART_FUNCTION,
    E::EASE_IN_QUINT_FUNCTION,    E::EASE_OUT_QUINT_FUNCTION,    E::EASE_IN_OUT_QUINT_FUNCTION,
    E::EASE_IN_EXPO_FUNCTION,     E::EASE_OUT_EXPO_FUNCTION,     E::EASE_IN_OUT_EXPO_FUNCTION,
    E::EASE_IN_CIRC_FUNCTION,     E::EASE_OUT_CIRC_FUNCTION,     E::EASE_IN_OUT_CIRC_FUNCTION,
    E::EASE_IN_BACK_FUNCTION,     E::EASE_OUT_BACK_FUNCTION,     E::EASE_IN_OUT_BACK_FUNCTION,
    E::EASE_IN_ELASTIC_FUNCTION,  E::EASE_OUT_ELASTIC_FUNCTION,  E::EASE_IN_OUT_ELASTIC_FUNCTION,
    E::EASE_IN_BOUNCE_FUNCTION,   E::EASE_OUT_BOUNCE_FUNCTION,   E::EASE_IN_OUT_BOUNCE_FUNCTION,
  };
  return TABLE[ease](t);
}

// ============================================================
//  DETERMINISTIC RANDOMNESS
// ============================================================

// Small integer hash (murmur3-style finalizer) combining a seed with a domain tag and a step
// id already stored on props (total_steps_completed / current_repetition), so the same
// (seed, domain, stepId) triple always reproduces the same draw within a step/pass but a new
// step/pass gets an independent one -- no persisted RNG engine required.
uint32_t LvsMorphCalculation::hashCombine(uint32_t seed, uint32_t domain, uint32_t stepId) {
  uint32_t h = seed * 2654435761u + domain * 40503u + stepId * 2246822519u;
  h ^= h >> 15; h *= 0x85ebca6bu;
  h ^= h >> 13; h *= 0xc2b2ae35u;
  h ^= h >> 16;
  return h;
}

float LvsMorphCalculation::sampleVariance(uint32_t seed, uint32_t domain, uint32_t stepId, float variance) {
  if (variance <= 0.0f) return 0.0f;
  std::mt19937 rng(hashCombine(seed, domain, stepId));
  std::uniform_real_distribution<float> dist(0.0f, variance);
  return dist(rng);
}

int LvsMorphCalculation::pickRandomIndex(int lo, int hi, int excluding, uint32_t seed, uint32_t stepId) {
  if (lo >= hi) return lo;
  std::mt19937 rng(hashCombine(seed, DOMAIN_RANDOM_INDEX, stepId));
  std::uniform_int_distribution<int> dist(lo, hi);
  int candidate = dist(rng);
  if (candidate == excluding) {
    candidate = (candidate == hi) ? lo : candidate + 1;
  }
  return candidate;
}

// ============================================================
//  SEQUENCING
// ============================================================

int LvsMorphCalculation::resolveEndIndex(const LvsMorph::morphProperties &props, int listSize) {
  return (props.end_index == -1) ? (listSize - 1) : props.end_index;
}

bool LvsMorphCalculation::advanceWithinPass(LvsMorph::morphProperties &props, int resolvedEnd, uint32_t effectiveSeed) {
  const int rangeLen = std::abs(resolvedEnd - props.start_index); // steps per pass; >=1, guaranteed by validation

  if (props.sequence_mode == MORPH_SEQUENCE_RANDOM) {
    // No positional notion of "the far end" exists for a random walk, so a pass is defined as
    // exactly rangeLen steps -- the same step count a FORWARD pass over the same range would
    // take -- purely so `repetition`/`step_delay`/`loop_delay`/`hold_at_end_duration` retain a
    // consistent meaning across every sequence_mode. This is a judgment call (the enum doc only
    // specifies "no immediate repeat"); flag to the user if a different semantic is wanted.
    props.current_from_index = props.current_to_index;
    props.current_to_index = pickRandomIndex(props.start_index, resolvedEnd, props.current_from_index,
                                              effectiveSeed, (uint32_t)props.total_steps_completed);
    int stepsIntoPass = props.total_steps_completed - props.current_repetition * rangeLen;
    return stepsIntoPass >= rangeLen;
  }

  int dir = (props.current_to_index > props.current_from_index) ? 1 : -1;
  bool atFarBoundary = (dir > 0) ? (props.current_to_index == resolvedEnd)
                                 : (props.current_to_index == props.start_index);

  // FORWARD / REVERSE: single-leg passes. Back-and-forth oscillation across repetitions is
  // handled entirely by reverse_on_finish in resetIndicesForNextPass, not here.
  if (atFarBoundary) return true;
  props.current_from_index = props.current_to_index;
  props.current_to_index   = props.current_to_index + dir;
  return false;
}

void LvsMorphCalculation::resetIndicesForNextPass(LvsMorph::morphProperties &props, int resolvedEnd, uint32_t effectiveSeed) {
  if (props.sequence_mode == MORPH_SEQUENCE_RANDOM) {
    props.current_from_index = props.start_index;
    props.current_to_index   = pickRandomIndex(props.start_index, resolvedEnd, props.start_index,
                                                effectiveSeed, (uint32_t)props.total_steps_completed);
    return;
  }

  // FORWARD / REVERSE: reverse_on_finish flips the base direction on alternating passes —
  // this is the only direction-reversal mechanism available now (see morphProperties::reverse_on_finish).
  bool baseForward = (props.sequence_mode == MORPH_SEQUENCE_FORWARD);
  bool nextForward = props.reverse_on_finish ? (baseForward == (props.current_repetition % 2 == 0)) : baseForward;

  if (nextForward) {
    props.current_from_index = props.start_index;
    props.current_to_index   = props.start_index + 1;
  } else {
    props.current_from_index = resolvedEnd;
    props.current_to_index   = resolvedEnd - 1;
  }
}

// ============================================================
//  VERTEX INTERPOLATION
// ============================================================

std::vector<LvsModel::Vertex> LvsMorphCalculation::matchByNearest(
    const std::vector<LvsModel::Vertex> &from, const std::vector<LvsModel::Vertex> &to) {
  std::vector<LvsModel::Vertex> result(to.size());
  for (size_t i = 0; i < to.size(); ++i) {
    size_t bestIdx = 0;
    float bestDist = std::numeric_limits<float>::max();
    for (size_t j = 0; j < from.size(); ++j) {
      glm::vec2 diff = to[i].position - from[j].position;
      float d = glm::dot(diff, diff);
      if (d < bestDist) { bestDist = d; bestIdx = j; }
    }
    result[i] = from[bestIdx];
  }
  return result;
}

std::vector<LvsModel::Vertex> LvsMorphCalculation::padToCount(const std::vector<LvsModel::Vertex> &shape, size_t targetCount) {
  if (shape.size() == targetCount) return shape;
  std::vector<LvsModel::Vertex> result;
  result.reserve(targetCount);
  if (shape.size() < targetCount) {
    result = shape;
    while (result.size() < targetCount) result.push_back(shape.back()); // expand: replicate last vertex
  } else {
    // Collapse: keep the first (targetCount - 1) vertices as-is, fold every remaining vertex
    // onto the final slot so extra geometry converges to a point instead of being discarded.
    result.assign(shape.begin(), shape.begin() + (targetCount - 1));
    result.push_back(shape.back());
  }
  return result;
}

std::vector<LvsModel::Vertex> LvsMorphCalculation::interpolateShapes(
    const std::vector<LvsModel::Vertex> &from, const std::vector<LvsModel::Vertex> &to,
    const LvsMorph::morphProperties &props, float posT, float colorT, float uvT) {

  std::vector<LvsModel::Vertex> pairedFrom;
  switch (props.vertex_match_mode) {
    case MORPH_VERTEX_BY_NEAREST:
      pairedFrom = matchByNearest(from, to);
      break;
    case MORPH_VERTEX_COLLAPSE_EXPAND:
      pairedFrom = padToCount(from, to.size());
      break;
    case MORPH_VERTEX_BY_INDEX:
    default:
      pairedFrom = from; // counts already guaranteed equal by checkMorphProperties
      break;
  }

  size_t n = std::min(pairedFrom.size(), to.size()); // defensive; should already be equal
  std::vector<LvsModel::Vertex> result(n);
  for (size_t i = 0; i < n; ++i) {
    result[i].position = glm::mix(pairedFrom[i].position, to[i].position, posT);
    result[i].color    = props.morph_vertex_colors ? glm::mix(pairedFrom[i].color, to[i].color, colorT) : to[i].color;
    result[i].uv       = props.morph_vertex_uvs    ? glm::mix(pairedFrom[i].uv,    to[i].uv,    uvT)    : to[i].uv;
  }
  return result;
}

// ============================================================
//  PUBLIC: PER-TICK DRIVER
// ============================================================

std::vector<LvsModel::Vertex> LvsMorphCalculation::calculateMorph(
    const std::vector<std::vector<LvsModel::Vertex>> &verticesList, LvsMorph::morphProperties &props, float deltaTime) {

  if (!props.active) return props.current_vertices;
  if (verticesList.empty()) return props.current_vertices; // defensive; checkMorphProperties should have rejected this

  const int listSize = static_cast<int>(verticesList.size());
  const int resolvedEnd = resolveEndIndex(props, listSize);

  // Effective RNG seed: the user-supplied seed if nonzero, otherwise a value derived from the
  // morph's target-object pointer. TARGET_OBJECT is required non-null and stable for the whole
  // morph's lifetime (checkMorphProperties rejects nullptr), and distinct morphs almost always
  // target distinct objects, so this gives each unseeded morph its own stable, independent
  // pseudo-random stream without needing any persisted RNG state.
  const uint32_t effectiveSeed = props.random_seed != 0
    ? props.random_seed
    : static_cast<uint32_t>(reinterpret_cast<uintptr_t>(props.TARGET_OBJECT));

  for (int iteration = 0; iteration < MAX_TICK_ITERATIONS; ++iteration) {
    switch (props.MORPH_STATE) {

      case MORPH_STATE_FINISHED:
      case MORPH_STATE_PAUSED:
      case MORPH_STATE_DELETED:
        return props.current_vertices;

      case MORPH_STATE_STARTING: {
        if (props.on_morph_start) props.on_morph_start(props.callback_data);

        props.current_from_index    = props.start_index;
        props.current_to_index      = (props.start_index < resolvedEnd) ? props.start_index + 1 : props.start_index - 1;
        props.current_repetition    = 0;
        props.total_steps_completed = 0;
        props.elapsed_time          = 0.0f;
        props.elapsed_delay_time    = 0.0f;
        props.delay_finished        = false;

        if (props.snap_to_start_immediately) {
          props.current_vertices    = verticesList[props.start_index];
          props.normalized_progress = 0.0f;
        }

        float sampledDelay = props.delay + sampleVariance(effectiveSeed, DOMAIN_DELAY, 0, props.delay_variance);
        if (sampledDelay > 0.0f) {
          props.MORPH_STATE = MORPH_STATE_DELAYING;
        } else {
          props.delay_finished = true;
          props.MORPH_STATE    = MORPH_STATE_PLAYING;
        }
        continue; // re-enter the loop immediately; no time consumed by this transition
      }

      case MORPH_STATE_DELAYING: {
        if (deltaTime <= 0.0f) return props.current_vertices;

        float sampledDelay = props.delay + sampleVariance(effectiveSeed, DOMAIN_DELAY, 0, props.delay_variance);
        float remaining    = sampledDelay - props.elapsed_delay_time;
        float step         = std::min(deltaTime, remaining);
        props.elapsed_delay_time += step;
        deltaTime -= step;

        if (props.elapsed_delay_time >= sampledDelay) {
          props.delay_finished = true;
          props.MORPH_STATE    = MORPH_STATE_PLAYING;
          continue;
        }
        return props.current_vertices;
      }

      case MORPH_STATE_PLAYING: {
        if (deltaTime <= 0.0f) return props.current_vertices;

        float sampledDuration = props.duration +
          sampleVariance(effectiveSeed, DOMAIN_DURATION, (uint32_t)props.total_steps_completed, props.duration_variance);
        sampledDuration = std::max(sampledDuration, 1e-5f); // duration is validated > 0; guards a degenerate sampled value

        float remaining = sampledDuration - props.elapsed_time;
        float step      = std::min(deltaTime, remaining);
        props.elapsed_time += step;
        deltaTime -= step;

        float rawT = std::clamp(props.elapsed_time / sampledDuration, 0.0f, 1.0f);
        props.normalized_progress = applyEase(props.EASE, props.MORPH_CUSTOM_EASE_FUNCTION, rawT);

        float posT   = applyEase(props.position_ease, props.position_custom_ease_function, rawT);
        float colorT = props.morph_vertex_colors ? applyEase(props.color_ease, props.color_custom_ease_function, rawT) : posT;
        float uvT    = props.morph_vertex_uvs    ? applyEase(props.uv_ease, props.uv_custom_ease_function, rawT)       : posT;

        props.current_vertices = interpolateShapes(
          verticesList[props.current_from_index], verticesList[props.current_to_index], props, posT, colorT, uvT);

        if (props.elapsed_time < sampledDuration) {
          return props.current_vertices; // deltaTime exhausted mid-step
        }

        // Step complete.
        props.total_steps_completed++;
        if (props.on_step_complete) props.on_step_complete(props.current_to_index, props.callback_data);
        props.elapsed_time = 0.0f;

        bool passExhausted = advanceWithinPass(props, resolvedEnd, effectiveSeed);

        if (!passExhausted) {
          float sampledStepDelay = props.step_delay +
            sampleVariance(effectiveSeed, DOMAIN_STEP_DELAY, (uint32_t)props.total_steps_completed, props.step_delay_variance);
          if (sampledStepDelay > 0.0f) {
            props.step_delay_remaining = sampledStepDelay;
            props.MORPH_STATE = MORPH_STATE_STEP_DELAY;
          } // else: stays PLAYING, loop continues straight into the next step
          continue;
        }

        // Pass exhausted: decide whether another repetition follows.
        props.current_repetition++;
        bool moreReps = (props.repetition == -1) || (props.current_repetition < props.repetition);

        // MORPH_STATE_LOOP_DELAY does double duty here: when moreReps is true it's the pause
        // between repetitions (loop_delay_remaining counts down props.loop_delay); when false
        // it's the hold on the final shape before finishing (loop_delay_remaining counts down
        // props.hold_at_end_duration instead). Both branches recompute `moreReps` identically
        // from current_repetition/repetition when the countdown completes, so no extra state
        // is needed to remember which purpose this particular hold is serving.
        float holdDuration = moreReps ? props.loop_delay : props.hold_at_end_duration;
        if (holdDuration > 0.0f) {
          props.loop_delay_remaining = holdDuration;
          props.MORPH_STATE = MORPH_STATE_LOOP_DELAY;
          continue;
        }

        if (moreReps) {
          resetIndicesForNextPass(props, resolvedEnd, effectiveSeed);
          props.MORPH_STATE = MORPH_STATE_PLAYING;
        } else {
          props.MORPH_STATE = MORPH_STATE_FINISHED;
          if (props.on_morph_complete) props.on_morph_complete(props.callback_data);
          // Freeing the slot (free_slots/active_indices) when destroy_on_finish is true is SoA
          // pool bookkeeping owned by LvsMorph, not this calculation class -- the not-yet-built
          // per-frame driver must check props.destroy_on_finish once MORPH_STATE_FINISHED is
          // observed and release the slot itself.
          return props.current_vertices;
        }
        continue;
      }

      case MORPH_STATE_STEP_DELAY: {
        if (deltaTime <= 0.0f) return props.current_vertices;

        float step = std::min(deltaTime, props.step_delay_remaining);
        props.step_delay_remaining -= step;
        deltaTime -= step;

        if (props.step_delay_remaining <= 0.0f) {
          props.step_delay_remaining = 0.0f;
          props.MORPH_STATE = MORPH_STATE_PLAYING;
          continue;
        }
        return props.current_vertices;
      }

      case MORPH_STATE_LOOP_DELAY: {
        if (deltaTime <= 0.0f) return props.current_vertices;

        float step = std::min(deltaTime, props.loop_delay_remaining);
        props.loop_delay_remaining -= step;
        deltaTime -= step;

        if (props.loop_delay_remaining > 0.0f) return props.current_vertices;
        props.loop_delay_remaining = 0.0f;

        bool moreReps = (props.repetition == -1) || (props.current_repetition < props.repetition);
        if (moreReps) {
          resetIndicesForNextPass(props, resolvedEnd, effectiveSeed);
          props.MORPH_STATE = MORPH_STATE_PLAYING;
        } else {
          props.MORPH_STATE = MORPH_STATE_FINISHED;
          if (props.on_morph_complete) props.on_morph_complete(props.callback_data);
          return props.current_vertices;
        }
        continue;
      }

      default:
        return props.current_vertices;
    }
  }

  std::cerr << cpc::Red << "[LvsMorphCalculation::calculateMorph] Exceeded " << MAX_TICK_ITERATIONS
    << " state-machine iterations in a single call (deltaTime = " << deltaTime << "). Returning current "
    "vertices without finishing this tick -- this indicates a pathologically large deltaTime or a "
    "degenerate morph configuration." << cpc::Reset << std::endl;
  return props.current_vertices;
}

}
