#pragma once

#include "../lvs_game_object.hpp"

// std:
#include <variant>
#include <vector>
#include <unordered_map>
#include <glm/gtc/constants.hpp>

namespace lvs {

class LvsGameObject;

using id_t = unsigned int;

class LvsGameAnimations {
public:
  enum AnimationType { ANIMATION_TYPE_ROTATION, ANIMATION_TYPE_TRANSLATION, ANIMATION_TYPE_SCALE };

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

  enum AnimationState {
    ANIMATION_STATE_DELAYING, ANIMATION_STATE_PLAYING, ANIMATION_STATE_FINISHED,
    ANIMATION_STATE_PAUSED, ANIMATION_STATE_DELETED, ANIMATION_STATE_STARTING
  };

  struct AnimationProperties {
    AnimationType TYPE;
    EaseType EASE{LINEAR};
    AnimationState ANIMATION_STATE;

    std::string ANIMATION_NAME;

    int TARGET_ID;

    float ANIMATION_DURATION{1.f}; // Seconds
    float ANIMATION_DELAY{0.f}; // Delay of animation in seconds
    float ANIMATION_ELAPSED_TIME{0.f}; // Tracks elapsed time in seconds
    float ANIMATION_ELAPSED_DELAY_TIME{0.f}; // Elapsed delay time
    int ANIMATION_REPETITION{1}; // -1 for infinite 0+ for specific count

    bool ANIMATION_DELAY_FINISHED{false};
    bool ANIMATION_REVERSE_ON_FINISH{false}; // A repetition counts only after completing the full round trip, not just upon reaching the endpoint.
    union {
      struct {
        float ANIMATION_STARTING_RADIAN;
        glm::vec2 PIVOT_POINT;
        float ANIMATION_ENDING_RADIAN;
        float RADIUS;
      } ANIMATION_TYPE_ROTATION;
      struct {
        glm::vec2 ANIMATION_STARTING_POSITION;
        float _PADDING;
        glm::vec2 ANIMATION_ENDING_POSITION;
        bool IS_RELATIVE;
      } ANIMATION_TYPE_TRANSLATION;
      struct {
        glm::vec2 ANIMATION_STARTING_SCALE_VECTOR;
        glm::vec2 ANIMATION_ENDING_SCALE_VECTOR;
      } ANIMATION_TYPE_SCALE;
    };

    void* CALLBACK_DATA{nullptr};
    float (*ANIMATION_CUSTOM_EASE_FUNCTION)(float){nullptr}; // Only for CUSTOM_CURVE_STEER from the EaseType
  };
  LvsGameAnimations() = default;
  ~LvsGameAnimations() = default;

  LvsGameAnimations(const LvsGameAnimations &) = delete;
  LvsGameAnimations& operator=(const LvsGameAnimations &) = delete;

  int setAnimation(AnimationProperties &animationProperties);
  void updateAnimations(std::unordered_map<id_t, LvsGameObject>& world);
  void pauseAnimation(int animationID);
  void continueAnimation(int animationID);
  AnimationProperties getAnimationInformation(int animationID);
private:
  typedef float (*AnimationEaseFunction)(float);
  static std::vector<AnimationProperties> activeAnimations;

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

  static const AnimationEaseFunction EASE_TABLE[];

  std::vector<AnimationType> ANIMATION_SOA_VECTOR_ANIMATION_TYPE;
  std::vector<EaseType> ANIMATION_SOA_VECTOR_ANIMATION_EASE;
  std::vector<AnimationState> ANIMATION_SOA_VECTOR_ANIMATION_STATE;

  std::vector<std::string> ANIMATION_SOA_VECTOR_ANIMATION_NAME;
  
  std::vector<int> ANIMATION_SOA_VECTOR_TARGET_ID;
  
  std::vector<float> ANIMATION_SOA_VECTOR_ANIMATION_DURATION;
  std::vector<float> ANIMATION_SOA_VECTOR_ANIMATION_DELAY;
  std::vector<float> ANIMATION_SOA_VECTOR_ANIMATION_ELAPSED_TIME;
  std::vector<float> ANIMATION_SOA_VECTOR_ANIMATION_ELAPSED_DELAY_TIME;
  std::vector<int> ANIMATION_SOA_VECTOR_ANIMATION_REPETITION;
  
  std::vector<bool> ANIMATION_SOA_VECTOR_ANIMATION_DELAY_FINISHED;
  std::vector<bool> ANIMATION_SOA_VECTOR_ANIMATION_REVERSE_ON_FINISH;

  std::vector<float> ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_STARTING_RADIAN;
  std::vector<glm::vec2> ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_PIVOT_POINT;
  std::vector<float> ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_ENDING_RADIAN;
  std::vector<float> ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_RADIUS;

  std::vector<glm::vec2> ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_STARTING_POSITION;
  std::vector<glm::vec2> ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_ENDING_POSITION;
  std::vector<bool> ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_IS_RELATIVE;

  std::vector<glm::vec2> ANIMATION_SOA_VECTOR_SCALING_ANIMATION_STARTING_SCALE_VECTOR;
  std::vector<glm::vec2> ANIMATION_SOA_VECTOR_SCALING_ANIMATION_ENDING_SCALE_VECTOR;

  std::vector<void*> ANIMATION_SOA_VECTOR_ANIMATION_CALLBACK_DATA;
  std::vector<float(*)(float)> ANIMATION_SOA_VECTOR_ANIMATION_CUSTOM_EASE_FUNCTION;

  std::vector<int> ANIMATION_SOA_VECTOR_ANIMATION_ACTIVE_INDICES;
  std::vector<int> ANIMATION_SOA_VECTOR_ANIMATION_FREE_SLOTS;

  std::unordered_map<id_t ,LvsGameObject>* GAME_OBJECTS;

  void setRotationAnimation(int animationID);
  void setTranslationAnimation(int animationID);
  void setScalingAnimation(int animationID);
  
  void checkArgumentValidity(int animationID, AnimationState animationState);
  void resetToOriginalState(int &animationID);
  
  glm::vec2 vectorChangeCalculation(glm::vec2 &startingVector, glm::vec2 &endingVector, int animationID);
  bool checkAnimationState(int animationID);

  void syncPropertiesWithSOA(int idx, AnimationProperties &props, bool writeToSOA);
  LvsGameObject* getGameObject(int targetID);
  bool hasAnimation(int &animationID);
};

extern LvsGameAnimations g_AnimationManager;
}