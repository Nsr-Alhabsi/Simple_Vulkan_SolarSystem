#include "lvs_game_animations.hpp"
#include "../lvs_game_object.hpp"
#include "../../vulkan_simulation.hpp"
#include "../../ADDONS/cp_color.hpp"

// std:
#include <iostream>
#include <algorithm>
#include <vector>
#include <cctype>

namespace lvs {

std::vector<LvsGameAnimations::AnimationProperties> LvsGameAnimations::activeAnimations;

const LvsGameAnimations::AnimationEaseFunction LvsGameAnimations::EASE_TABLE[] = {
  LvsEasingFunctions::LINEAR_FUNCTION,
  LvsEasingFunctions::EASE_IN_SINE_FUNCTION,
    LvsEasingFunctions::EASE_OUT_SINE_FUNCTION,
      LvsEasingFunctions::EASE_IN_OUT_SINE_FUNCTION,
  LvsEasingFunctions::EASE_IN_QUAD_FUNCTION,
    LvsEasingFunctions::EASE_OUT_QUAD_FUNCTION,
      LvsEasingFunctions::EASE_IN_OUT_QUAD_FUNCTION,
  LvsEasingFunctions::EASE_IN_CUBIC_FUNCTION,
    LvsEasingFunctions::EASE_OUT_CUBIC_FUNCTION,
      LvsEasingFunctions::EASE_IN_OUT_CUBIC_FUNCTION,
  LvsEasingFunctions::EASE_IN_QUART_FUNCTION,
    LvsEasingFunctions::EASE_OUT_QUART_FUNCTION,
      LvsEasingFunctions::EASE_IN_OUT_QUART_FUNCTION,
  LvsEasingFunctions::EASE_IN_QUINT_FUNCTION,
    LvsEasingFunctions::EASE_OUT_QUINT_FUNCTION,
      LvsEasingFunctions::EASE_IN_OUT_QUINT_FUNCTION,
  LvsEasingFunctions::EASE_IN_EXPO_FUNCTION,
    LvsEasingFunctions::EASE_OUT_EXPO_FUNCTION,
      LvsEasingFunctions::EASE_IN_OUT_EXPO_FUNCTION,
  LvsEasingFunctions::EASE_IN_CIRC_FUNCTION,
    LvsEasingFunctions::EASE_OUT_CIRC_FUNCTION,
      LvsEasingFunctions::EASE_IN_OUT_CIRC_FUNCTION,
  LvsEasingFunctions::EASE_IN_BACK_FUNCTION,
    LvsEasingFunctions::EASE_OUT_BACK_FUNCTION,
      LvsEasingFunctions::EASE_IN_OUT_BACK_FUNCTION,
  LvsEasingFunctions::EASE_IN_ELASTIC_FUNCTION,
    LvsEasingFunctions::EASE_OUT_ELASTIC_FUNCTION,
      LvsEasingFunctions::EASE_IN_OUT_ELASTIC_FUNCTION,
  LvsEasingFunctions::EASE_IN_BOUNCE_FUNCTION,
    LvsEasingFunctions::EASE_OUT_BOUNCE_FUNCTION,
      LvsEasingFunctions::EASE_IN_OUT_BOUNCE_FUNCTION
};

LvsGameObject* LvsGameAnimations::getGameObject(int targetID) {
  if (!GAME_OBJECTS) throw std::runtime_error("INVALID OBJECT: No objects inside of object list");
  LvsGameObject* target = nullptr;
  for (auto& obj : *GAME_OBJECTS) {
    if (obj.second.getId() == (unsigned int)targetID) {
      target = &obj.second;
      break;
    }
  }
  return target;
}

void LvsGameAnimations::setRotationAnimation(int animationID) {
  float normalizedProgress = soa.vector_animation_elapsed_time[animationID] / soa.vector_animation_duration[animationID];
  if (normalizedProgress > 1.0f) normalizedProgress = 1.0f;

  if (soa.vector_animation_reverse_on_finish[animationID]) {
    normalizedProgress = (normalizedProgress <= 0.5f)
    ? (normalizedProgress * 2.0f)
    : (2.0f - (normalizedProgress * 2.0f));
  }

  float easedInterpolationFactor = (soa.vector_animation_custom_ease_function[animationID] == nullptr)
  ? EASE_TABLE[soa.vector_animation_ease[animationID]](normalizedProgress)
  : soa.vector_animation_custom_ease_function[animationID](normalizedProgress);

  float startingRadian = soa.vector_animation_starting_radian[animationID];
  float endingRadian = soa.vector_animation_ending_radian[animationID];

  float totalRotationDistance = endingRadian - startingRadian;
  float currentFrameRotation = startingRadian + (totalRotationDistance * easedInterpolationFactor);

  glm::vec2 pivot = soa.vector_animation_pivot_point[animationID];

  float radius = soa.vector_animation_radius[animationID];

  float x = pivot.x + cosf(currentFrameRotation) * radius;
  float y = pivot.y + sinf(currentFrameRotation) * radius;

  auto gameObject = getGameObject(soa.vector_target_ID[animationID]);

  gameObject->transform2D.translation = {x, y};
  gameObject->transform2D.rotation = currentFrameRotation;
}

glm::vec2 LvsGameAnimations::vectorChangeCalculation(glm::vec2 &startingVector, glm::vec2 &endingVector, int animationID) {
  float animationTimeProgess = (
    soa.vector_animation_elapsed_time[animationID] / soa.vector_animation_duration[animationID]
  );
  if (animationTimeProgess > 1.0f) animationTimeProgess = 1.0f;

  if (soa.vector_animation_reverse_on_finish[animationID]) {
    animationTimeProgess = (animationTimeProgess <= 0.5f)
    ? (animationTimeProgess * 2.0f)
    : (2.0f - (animationTimeProgess * 2.0f));
  }

  float easedInterpolationFactor = (soa.vector_animation_custom_ease_function[animationID] == nullptr)
  ? EASE_TABLE[soa.vector_animation_ease[animationID]](animationTimeProgess)
  : soa.vector_animation_custom_ease_function[animationID](animationTimeProgess);

  if (soa.vector_animation_is_relative[animationID]) {
    endingVector = startingVector + endingVector;
  }

  float totalXValue = endingVector.x - startingVector.x;
  float totalYValue = endingVector.y - startingVector.y;

  float currentFrameVectorX = startingVector.x + (totalXValue * easedInterpolationFactor);
  float currentFrameVectorY = startingVector.y + (totalYValue * easedInterpolationFactor);

  return {currentFrameVectorX, currentFrameVectorY};
}

void LvsGameAnimations::setTranslationAnimation(int animationID) {
  glm::vec2 currentFramePosition = vectorChangeCalculation(
    soa.vector_animation_starting_position[animationID],
    soa.vector_animation_ending_position[animationID],
    animationID
  );

  auto gameObject = getGameObject(soa.vector_target_ID[animationID]);

  gameObject->transform2D.translation.x = currentFramePosition.x;
  gameObject->transform2D.translation.y = currentFramePosition.y;
}

void LvsGameAnimations::setScalingAnimation(int animationID) {
  glm::vec2 currentFrameScale = vectorChangeCalculation(
    soa.vector_animation_starting_scale[animationID],
    soa.vector_animation_ending_scale[animationID],
    animationID
  );

  auto gameObject = getGameObject(soa.vector_target_ID[animationID]);

  gameObject->transform2D.scale.x = currentFrameScale.x;
  gameObject->transform2D.scale.y = currentFrameScale.y;
}

bool LvsGameAnimations::hasAnimation(int &animationID) {
  if (animationID < 0) return false;
  if (static_cast<size_t>(animationID) < soa.vector_animation_name.size()) {
    return soa.vector_animation_state[animationID] != AnimationState::ANIMATION_STATE_FINISHED;
  }
  return false;
}

LvsGameAnimations::AnimationProperties LvsGameAnimations::getAnimationInformation(int animationID) {
  if (!hasAnimation(animationID)) throw std::runtime_error("INVALID ANIMATION ID: Could not find animation property with ID: " + std::to_string(animationID));
  AnimationProperties props;
  syncPropertiesWithSOA(animationID, props, false);
  return props;
}

void LvsGameAnimations::syncPropertiesWithSOA(int idx, AnimationProperties &props, bool writeToSOA) {
#define SYNC_VAL(field, vector) if(writeToSOA) vector[idx] = field; else field = static_cast<decltype(field)>(vector[idx]);
#define SYNC_PUSH(field, vector) if(writeToSOA) vector.push_back(field);

  if (writeToSOA && (size_t)idx == soa.vector_animation_type.size()) {
    SYNC_PUSH(props.TYPE, soa.vector_animation_type);
    SYNC_PUSH(props.EASE, soa.vector_animation_ease);
    SYNC_PUSH(AnimationState::ANIMATION_STATE_STARTING, soa.vector_animation_state);
    SYNC_PUSH(props.ANIMATION_NAME, soa.vector_animation_name);
    SYNC_PUSH(props.TARGET_ID, soa.vector_target_ID);
    SYNC_PUSH(props.duration, soa.vector_animation_duration);
    SYNC_PUSH(props.delay, soa.vector_animation_delay);
    SYNC_PUSH(0.f, soa.vector_animation_elapsed_time);
    SYNC_PUSH(0.f, soa.vector_animation_elapsed_delay_time);
    SYNC_PUSH(props.repetition, soa.vector_animation_repetition);
    SYNC_PUSH(false, soa.vector_animation_delay_finished);
    SYNC_PUSH(props.reverse_on_finish, soa.vector_animation_reverse_on_finish);
    SYNC_PUSH(props.CALLBACK_DATA, soa.vector_animation_callback);
    SYNC_PUSH(props.ANIMATION_CUSTOM_EASE_FUNCTION, soa.vector_animation_custom_ease_function);

    if (props.TYPE == ANIMATION_TYPE_ROTATION) {
      SYNC_PUSH(props.rotation.starting_radian, soa.vector_animation_starting_radian);
      SYNC_PUSH(props.rotation.pivot_point, soa.vector_animation_pivot_point);
      SYNC_PUSH(props.rotation.ending_radian, soa.vector_animation_ending_radian);
      SYNC_PUSH(props.rotation.radius, soa.vector_animation_radius);
    } else {
      SYNC_PUSH(0.f, soa.vector_animation_starting_radian);
      SYNC_PUSH(glm::vec2(0.f), soa.vector_animation_pivot_point);
      SYNC_PUSH(0.f, soa.vector_animation_ending_radian);
      SYNC_PUSH(1.f, soa.vector_animation_radius);
    }

    if (props.TYPE == ANIMATION_TYPE_TRANSLATION) {
      SYNC_PUSH(props.translation.starting_position, soa.vector_animation_starting_position);
      SYNC_PUSH(props.translation.ending_position, soa.vector_animation_ending_position);
      SYNC_PUSH(props.translation.is_relative, soa.vector_animation_is_relative);
    } else {
      SYNC_PUSH(glm::vec2(0.f), soa.vector_animation_starting_position);
      SYNC_PUSH(glm::vec2(0.f), soa.vector_animation_ending_position);
      SYNC_PUSH(false, soa.vector_animation_is_relative);
    }

    if (props.TYPE == ANIMATION_TYPE_SCALE) {
      SYNC_PUSH(props.scale.starting_scale, soa.vector_animation_starting_scale);
      SYNC_PUSH(props.scale.ending_scale, soa.vector_animation_ending_scale);
    } else {
      SYNC_PUSH(glm::vec2(1.f), soa.vector_animation_starting_scale);
      SYNC_PUSH(glm::vec2(1.f), soa.vector_animation_ending_scale);
    }
  } else {
    SYNC_VAL(props.TYPE, soa.vector_animation_type);
    SYNC_VAL(props.EASE, soa.vector_animation_ease);
    SYNC_VAL(props.ANIMATION_STATE, soa.vector_animation_state);
    SYNC_VAL(props.ANIMATION_NAME, soa.vector_animation_name);
    SYNC_VAL(props.TARGET_ID, soa.vector_target_ID);
    SYNC_VAL(props.duration, soa.vector_animation_duration);
    SYNC_VAL(props.delay, soa.vector_animation_delay);
    SYNC_VAL(props.elapsed_time, soa.vector_animation_elapsed_time);
    SYNC_VAL(props.elapsed_delay_time, soa.vector_animation_elapsed_delay_time);
    SYNC_VAL(props.repetition, soa.vector_animation_repetition);
    SYNC_VAL(props.delay_finished, soa.vector_animation_delay_finished);
    SYNC_VAL(props.reverse_on_finish, soa.vector_animation_reverse_on_finish);
    SYNC_VAL(props.CALLBACK_DATA, soa.vector_animation_callback);
    SYNC_VAL(props.ANIMATION_CUSTOM_EASE_FUNCTION, soa.vector_animation_custom_ease_function);

    if (writeToSOA) {
      if (props.TYPE == ANIMATION_TYPE_ROTATION) {
        soa.vector_animation_starting_radian[idx] = props.rotation.starting_radian;
        soa.vector_animation_pivot_point[idx] = props.rotation.pivot_point;
        soa.vector_animation_ending_radian[idx] = props.rotation.ending_radian;
        soa.vector_animation_radius[idx] = props.rotation.radius;
      } else if (props.TYPE == ANIMATION_TYPE_TRANSLATION) {
        soa.vector_animation_starting_position[idx] = props.translation.starting_position;
        soa.vector_animation_ending_position[idx] = props.translation.ending_position;
        soa.vector_animation_is_relative[idx] = props.translation.is_relative;
      } else if (props.TYPE == ANIMATION_TYPE_SCALE) {
        soa.vector_animation_starting_scale[idx] = props.scale.starting_scale;
        soa.vector_animation_ending_scale[idx] = props.scale.ending_scale;
      }
    } else {
      props.rotation.starting_radian = soa.vector_animation_starting_radian[idx];
      props.rotation.pivot_point = soa.vector_animation_pivot_point[idx];
      props.rotation.ending_radian = soa.vector_animation_ending_radian[idx];
      props.rotation.radius = soa.vector_animation_radius[idx];
      props.translation.starting_position = soa.vector_animation_starting_position[idx];
      props.translation.ending_position = soa.vector_animation_ending_position[idx];
      props.translation.is_relative = soa.vector_animation_is_relative[idx];
      props.scale.starting_scale = soa.vector_animation_starting_scale[idx];
      props.scale.ending_scale = soa.vector_animation_ending_scale[idx];
    }
  }
#undef SYNC_VAL
#undef SYNC_PUSH
}

int LvsGameAnimations::setAnimation(AnimationProperties &animationProperties) {
  std::cout << cpc::Cyan << "Creating Animation..." << cpc::Reset << std::endl;

  bool isOnlyWhitespace = std::all_of(animationProperties.ANIMATION_NAME.begin(), animationProperties.ANIMATION_NAME.end(), [](unsigned char ch) { return std::isspace(ch); });
  if (animationProperties.ANIMATION_NAME.empty() || isOnlyWhitespace) {
    throw std::runtime_error("RUNTIME ERROR: Animation name cannot be empty or contain only spaces!");
  }

  int collisionCount = 0;
  for (const auto& existingAnim : activeAnimations) {
    if (existingAnim.ANIMATION_NAME.find(animationProperties.ANIMATION_NAME) != std::string::npos) {
      collisionCount++;
    }
  }

  if (collisionCount > 0) {
    std::string oldName = animationProperties.ANIMATION_NAME;
    animationProperties.ANIMATION_NAME += std::to_string(collisionCount);

    std::cout << cpc::Yellow << "WARNING: Animation name of: {" << oldName << "} matches with another animation name | " <<
      "New Animation name setted to: {" << animationProperties.ANIMATION_NAME << "}" << cpc::Reset << std::endl;
  }

  if (animationProperties.duration < 0) { throw std::runtime_error("Animation Duration can't be less then zero"); }
  if (animationProperties.delay < 0) { throw std::runtime_error("Animation Delay can't be less than zero"); }

  animationProperties.delay_finished = (animationProperties.delay <= animationProperties.elapsed_delay_time);

  int idxToInsertIn;
  if (!soa.ANIMATION_SOA_VECTOR_ANIMATION_FREE_SLOTS.empty()) {
    idxToInsertIn = soa.ANIMATION_SOA_VECTOR_ANIMATION_FREE_SLOTS.back();
    soa.ANIMATION_SOA_VECTOR_ANIMATION_FREE_SLOTS.pop_back();
  } else {
    idxToInsertIn = static_cast<int>(soa.vector_animation_type.size());
  }

  syncPropertiesWithSOA(idxToInsertIn, animationProperties, true);

  soa.ANIMATION_SOA_VECTOR_ANIMATION_ACTIVE_INDICES.push_back(idxToInsertIn);

  std::cout << cpc::Green << "Successfully created animation with animation name: " << animationProperties.ANIMATION_NAME << cpc::Reset << std::endl;
  activeAnimations.push_back(std::move(animationProperties));

  return idxToInsertIn;
}

void LvsGameAnimations::checkArgumentValidity(int animationID, AnimationState animationState) {
  if (hasAnimation(animationID)) {
    if (soa.vector_animation_state[animationID] == animationState) return;
    soa.vector_animation_state[animationID] = animationState;
  } else {
    std::cout << cpc::Yellow << "Did not find a animation that matches with id: "
              << animationID << cpc::Reset << std::endl;
  }
}

void LvsGameAnimations::pauseAnimation(int animationID) {
  checkArgumentValidity(animationID, ANIMATION_STATE_PAUSED);
}

void LvsGameAnimations::continueAnimation(int animationID) {
  checkArgumentValidity(animationID, ANIMATION_STATE_DELAYING); 
}

void LvsGameAnimations::resetToOriginalState(int &animationID) {
  auto gameObject = getGameObject(soa.vector_target_ID[animationID]);

  switch(soa.vector_animation_type[animationID]) {
  case ANIMATION_TYPE_ROTATION:
    gameObject->transform2D.rotation = soa.vector_animation_starting_radian[animationID];
    break;
  case ANIMATION_TYPE_SCALE:
    gameObject->transform2D.scale = soa.vector_animation_starting_scale[animationID];
    break;
  case ANIMATION_TYPE_TRANSLATION:
    gameObject->transform2D.translation = soa.vector_animation_starting_position[animationID];
    break;
  }
}

bool LvsGameAnimations::checkAnimationState(int animationID)  {
  int& animationState              = soa.vector_animation_state[animationID];
  float& animationElapsedDelayTime = soa.vector_animation_elapsed_delay_time[animationID];
  float& animationElapsedTime      = soa.vector_animation_elapsed_time[animationID];
  float& animationDuration         = soa.vector_animation_duration[animationID];
  float& animationDelay            = soa.vector_animation_delay[animationID];
  int& animationRepetition         = soa.vector_animation_repetition[animationID];
  auto animationDelayFinished      = soa.vector_animation_delay_finished[animationID];


  if (animationState == ANIMATION_STATE_PAUSED) {
    return true;
  }

  if (!animationDelayFinished) {
    if (animationState != ANIMATION_STATE_DELAYING) animationState = ANIMATION_STATE_DELAYING;

    animationElapsedDelayTime += Simulation::FrameInformation.deltaFrameTime;

    animationDelayFinished = (animationDelay <= animationElapsedDelayTime);
    if (!animationDelayFinished) {
      return true;
    }

    animationElapsedDelayTime = (animationElapsedDelayTime - animationDelay);
  }

  if (animationState != ANIMATION_STATE_PLAYING) {
    animationState = ANIMATION_STATE_PLAYING;
  }

  animationElapsedTime += Simulation::FrameInformation.deltaFrameTime;


  if (animationElapsedTime >= animationDuration) {
    if (animationRepetition > 1 || animationRepetition == -1) {
      if (animationRepetition != -1) animationRepetition--;
      resetToOriginalState(animationID);
      animationElapsedTime -= animationDuration;
      return false;
    }

    if (animationState != ANIMATION_STATE_FINISHED) {
      animationState = ANIMATION_STATE_FINISHED;
      animationElapsedTime = animationDuration;
      return true; 
    }

    std::cout << cpc::Cyan << "Cleaning up finished animaiton: " << soa.vector_animation_name[animationID] << cpc::Reset << std::endl;

    std::vector<int>& activeIndicies = soa.ANIMATION_SOA_VECTOR_ANIMATION_ACTIVE_INDICES;
    int foundIndexInActive = -1;
    for (int i = 0; i < (int)activeIndicies.size(); i++) {
      if (activeIndicies[i] == animationID) {
        foundIndexInActive = i;
        break;
      }
    }

    if (foundIndexInActive != -1) {
      activeIndicies[foundIndexInActive] = activeIndicies.back();
      activeIndicies.pop_back();

      animationState = ANIMATION_STATE_DELETED;
      soa.ANIMATION_SOA_VECTOR_ANIMATION_FREE_SLOTS.push_back(animationID);

      std::cout << cpc::Cyan << "Recycled Slot: " << animationID << ". Active count: " << activeIndicies.size() << cpc::Reset << std::endl; 
    }
    return true;
  }

  return false;
}

void LvsGameAnimations::updateAnimations(std::unordered_map<LvsGameObject::id_t, LvsGameObject>& world) {
  GAME_OBJECTS = &world;

  for (int idx : soa.ANIMATION_SOA_VECTOR_ANIMATION_ACTIVE_INDICES) {
    if (checkAnimationState(idx)) continue;

    switch (soa.vector_animation_type[idx]) {
      case ANIMATION_TYPE_ROTATION:
        setRotationAnimation(idx);
        break;
      case ANIMATION_TYPE_TRANSLATION:
        setTranslationAnimation(idx);
        break;
      case ANIMATION_TYPE_SCALE:
        setScalingAnimation(idx);
        break;
    }
  }
}

}