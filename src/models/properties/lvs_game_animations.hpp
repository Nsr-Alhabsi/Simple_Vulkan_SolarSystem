#pragma once

#include "../lvs_game_object.hpp"
#include "../../utils/easing_functions.hpp"

// std:
#include <unordered_map>

namespace lvs {

class LvsGameObject;

using id_t = unsigned int;

class LvsGameAnimations {
public:
  enum AnimationType { ANIMATION_TYPE_ROTATION, ANIMATION_TYPE_TRANSLATION, ANIMATION_TYPE_SCALE };

  enum AnimationState {
    ANIMATION_STATE_DELAYING, ANIMATION_STATE_PLAYING, ANIMATION_STATE_FINISHED,
    ANIMATION_STATE_PAUSED, ANIMATION_STATE_DELETED, ANIMATION_STATE_STARTING
  };

  struct AnimationProperties {
    AnimationType TYPE;
    LvsEasingFunctions::EaseType EASE{LvsEasingFunctions::LINEAR};
    AnimationState ANIMATION_STATE;

    std::string ANIMATION_NAME;

    int TARGET_ID;

    float duration{1.f}; // Seconds
    float delay{0.f}; // Delay of animation in seconds
    float elapsed_time{0.f}; // Tracks elapsed time in seconds
    float elapsed_delay_time{0.f}; // Elapsed delay time
    int repetition{1}; // -1 for infinite 0+ for specific count

    bool delay_finished{false};
    bool reverse_on_finish{false}; // A repetition counts only after completing the full round trip, not just upon reaching the endpoint.
    union {
      struct {
        float starting_radian;
        glm::vec2 pivot_point;
        float ending_radian;
        float radius;
      } rotation;
      struct {
        glm::vec2 starting_position;
        float _PADDING;
        glm::vec2 ending_position;
        bool is_relative;
      } translation;
      struct {
        glm::vec2 starting_scale;
        glm::vec2 ending_scale;
      } scale;
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

  static const AnimationEaseFunction EASE_TABLE[];

  std::vector<AnimationType> ANIMATION_SOA_VECTOR_ANIMATION_TYPE;
  std::vector<LvsEasingFunctions::EaseType> ANIMATION_SOA_VECTOR_ANIMATION_EASE;
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