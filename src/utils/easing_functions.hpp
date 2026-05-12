#pragma once

// libs:
#include <glm/gtc/constants.hpp>
#include <math.h>

namespace lvs {

class LvsEasingFunctions {
public:
  enum EaseType {
    LINEAR,
    EASE_IN_SINE, EASE_OUT_SINE, EASE_IN_OUT_SINE,
    EASE_IN_QUAD, EASE_OUT_QUAD, EASE_IN_OUT_QUAD,
    EASE_IN_CUBIC, EASE_OUT_CUBIC, EASE_IN_OUT_QUBIC,
    EASE_IN_QUART, EASE_OUT_QUART, EASE_IN_OUT_QUART,
    EASE_IN_QUINT, EASE_OUT_QUINT, EASE_IN_OUT_QUINT,
    EASE_IN_EXPO, EASE_OUT_EXPO, EASE_IN_OUT_EXPO,
    EASE_IN_CIRC, EASE_OUT_CIRC, EASE_IN_OUT_CIRC,
    EASE_IN_BACK, EASE_OUT_BACK, EASE_IN_OUT_BACK,
    EASE_IN_ELASTIC, EASE_OUT_ELASTIC, EASE_IN_OUT_ELASTIC,
    EASE_IN_BOUNCE, EASE_OUT_BOUNCE, EASE_IN_OUT_BOUNCE,
    CUSTOM_CURVE_STEER
  };

  static float LINEAR_FUNCTION(float t) { return t; }
  static float EASE_IN_SINE_FUNCTION(float t) { return 1.0f - cosf(t * (glm::pi<float>() / 2)); }
  static float EASE_OUT_SINE_FUNCTION(float t) { return sinf(t * (glm::pi<float>() / 2)); }
  static float EASE_IN_OUT_SINE_FUNCTION(float t) { return -(cosf(glm::pi<float>() * t) - 1.0f) / 2.0f; }
  static float EASE_IN_QUAD_FUNCTION(float t) { return t * t; }
  static float EASE_OUT_QUAD_FUNCTION(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }
  static float EASE_IN_OUT_QUAD_FUNCTION(float t) { return t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f; }
  static float EASE_IN_CUBIC_FUNCTION(float t) { return t * t * t; }
  static float EASE_OUT_CUBIC_FUNCTION(float t) { 
    float f = (t - 1.0f);
    return f * f * f + 1.0f;
  }
  static float EASE_IN_OUT_CUBIC_FUNCTION(float t) {return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f; }
  static float EASE_IN_QUART_FUNCTION(float t) { return t * t * t * t; }
  static float EASE_OUT_QUART_FUNCTION(float t) {
    float f = (t - 1.0f);
    return 1.0f - (f * f * f * f);
  }
  static float EASE_IN_OUT_QUART_FUNCTION(float t) { return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 4.0f) / 2.0f; }
  static float EASE_IN_QUINT_FUNCTION(float t) { return t * t * t * t * t; }
  static float EASE_OUT_QUINT_FUNCTION(float t) { 
    float f = (t - 1.0f);
    return f * f * f * f * f + 1.0f;
  }
  static float EASE_IN_OUT_QUINT_FUNCTION(float t) { return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 5.0f) / 2.0f; }
  static float EASE_IN_EXPO_FUNCTION(float t) { return (t == 0.0f) ? 0.0f : powf(2.0f, 10.0f * t - 10.0f); }
  static float EASE_OUT_EXPO_FUNCTION(float t) { return (t == 1.0f) ? 1.0f : 1.0f - powf(2.0f, -10.0f * t); }
  static float EASE_IN_OUT_EXPO_FUNCTION(float t) { 
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    if (t < 0.5f) return powf(2.0f, 20.0f * t - 10.0f) / 2.0f;
    return (2.0f - powf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
  }
  static float EASE_IN_CIRC_FUNCTION(float t) { return 1.0f - sqrtf(1.0f - (t * t)); }
  static float EASE_OUT_CIRC_FUNCTION(float t) { 
    float f = (t - 1.0f);
    return sqrtf(1.0f - (f * f));
  }
  static float EASE_IN_OUT_CIRC_FUNCTION(float t) {
    if (t < 0.5f) return (1.0f - sqrtf(1.0f - (2.0f * t) * (2.0f * t))) / 2.0f;
    float f = (-2.0f * t + 2.0f);
    return (sqrtf(1.0f - (f * f)) + 1.0f) / 2.0f;
  }
  static float EASE_IN_BACK_FUNCTION(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return c3 * t * t * t - c1 * t * t;
  }
  static float EASE_OUT_BACK_FUNCTION(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    float f = t - 1.0f;
    return 1.0f + c3 * (f * f * f) + c1 * (f * f);
  }
  static float EASE_IN_OUT_BACK_FUNCTION(float t) {
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f; 
    return t < 0.5f
      ? (powf(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f
      : (powf(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
  }
  static float EASE_IN_ELASTIC_FUNCTION(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    const float c4 = (2.0f * glm::pi<float>()) / 3.0f;
    return -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * c4);
  }
  static float EASE_OUT_ELASTIC_FUNCTION(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    const float c4 = (2.0f * glm::pi<float>()) / 3.0f;
    return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
  }
  static float EASE_IN_OUT_ELASTIC_FUNCTION(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    const float c5 = (2.0f * glm::pi<float>()) / 4.5f;
    if (t < 0.5f) {
      return -(powf(2.0f, 20.0f * t - 10.0f) * sinf((20.0f * t - 11.125f) * c5)) / 2.0f;
    } else {
      return (powf(2.0f, -20.0f * t + 10.0f) * sinf((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
    }
  }
  static float EASE_OUT_BOUNCE_FUNCTION(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;

    if (t < 1.0f / d1) {
      return n1 * t * t;
    } else if (t < 2.0f / d1) {
      float f = t - (1.5f / d1);
      return n1 * f * f + 0.75f;
    } else if (t < 2.5f / d1) {
      float f = t - (2.25f / d1);
      return n1 * f * f + 0.9375f;
    } else {
      float f = t - (2.625f / d1);
      return n1 * f * f + 0.984375f;
    }
  }
  static float EASE_IN_BOUNCE_FUNCTION(float t) { return 1.0f - EASE_OUT_BOUNCE_FUNCTION(1.0f - t); }
  static float EASE_IN_OUT_BOUNCE_FUNCTION(float t) {
    return t < 0.5f
      ? (1.0f - EASE_OUT_BOUNCE_FUNCTION(1.0f - 2.0f * t)) / 2.0f
      : (1.0f + EASE_OUT_BOUNCE_FUNCTION(2.0f * t - 1.0f)) / 2.0f;
  }
};

}