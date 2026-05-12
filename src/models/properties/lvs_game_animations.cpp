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
  LvsGameAnimations::LINEAR_FUNCTION,
  LvsGameAnimations::EASE_IN_SINE_FUNCTION,
    LvsGameAnimations::EASE_OUT_SINE_FUNCTION,
      LvsGameAnimations::EASE_IN_OUT_SINE_FUNCTION,
  LvsGameAnimations::EASE_IN_QUAD_FUNCTION,
    LvsGameAnimations::EASE_OUT_QUAD_FUNCTION,
      LvsGameAnimations::EASE_IN_OUT_QUAD_FUNCTION,
  LvsGameAnimations::EASE_IN_CUBIC_FUNCTION,
    LvsGameAnimations::EASE_OUT_CUBIC_FUNCTION,
      LvsGameAnimations::EASE_IN_OUT_CUBIC_FUNCTION,
  LvsGameAnimations::EASE_IN_QUART_FUNCTION,
    LvsGameAnimations::EASE_OUT_QUART_FUNCTION,
      LvsGameAnimations::EASE_IN_OUT_QUART_FUNCTION,
  LvsGameAnimations::EASE_IN_QUINT_FUNCTION,
    LvsGameAnimations::EASE_OUT_QUINT_FUNCTION,
      LvsGameAnimations::EASE_IN_OUT_QUINT_FUNCTION,
  LvsGameAnimations::EASE_IN_EXPO_FUNCTION,
    LvsGameAnimations::EASE_OUT_EXPO_FUNCTION,
      LvsGameAnimations::EASE_IN_OUT_EXPO_FUNCTION,
  LvsGameAnimations::EASE_IN_CIRC_FUNCTION,
    LvsGameAnimations::EASE_OUT_CIRC_FUNCTION,
      LvsGameAnimations::EASE_IN_OUT_CIRC_FUNCTION,
  LvsGameAnimations::EASE_IN_BACK_FUNCTION,
    LvsGameAnimations::EASE_OUT_BACK_FUNCTION,
      LvsGameAnimations::EASE_IN_OUT_BACK_FUNCTION,
  LvsGameAnimations::EASE_IN_ELASTIC_FUNCTION,
    LvsGameAnimations::EASE_OUT_ELASTIC_FUNCTION,
      LvsGameAnimations::EASE_IN_OUT_ELASTIC_FUNCTION,
  LvsGameAnimations::EASE_IN_BOUNCE_FUNCTION,
    LvsGameAnimations::EASE_OUT_BOUNCE_FUNCTION,
      LvsGameAnimations::EASE_IN_OUT_BOUNCE_FUNCTION
};

LvsGameObject* LvsGameAnimations::getGameObject(int targetID) {
  if (!GAME_OBJECTS) throw std::runtime_error("INVALID OBJECT: No objects inside of object list");
  LvsGameObject* target = nullptr;
  for (auto& obj : *GAME_OBJECTS) {
    if (obj.second.getId() == targetID) {
      target = &obj.second;
      break;
    }
  }
  return target;
}

void LvsGameAnimations::setRotationAnimation(int animationID) {
  float normalizedProgress = ANIMATION_SOA_VECTOR_ANIMATION_ELAPSED_TIME[animationID] / ANIMATION_SOA_VECTOR_ANIMATION_DURATION[animationID];
  if (normalizedProgress > 1.0f) normalizedProgress = 1.0f;

  if (ANIMATION_SOA_VECTOR_ANIMATION_REVERSE_ON_FINISH[animationID]) {
    normalizedProgress = (normalizedProgress <= 0.5f)
    ? (normalizedProgress * 2.0f)
    : (2.0f - (normalizedProgress * 2.0f));
  }

  float easedInterpolationFactor = (ANIMATION_SOA_VECTOR_ANIMATION_CUSTOM_EASE_FUNCTION[animationID] == nullptr) 
  ? EASE_TABLE[ANIMATION_SOA_VECTOR_ANIMATION_EASE[animationID]](normalizedProgress) 
  : ANIMATION_SOA_VECTOR_ANIMATION_CUSTOM_EASE_FUNCTION[animationID](normalizedProgress);

  float startingRadian = ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_STARTING_RADIAN[animationID];
  float endingRadian = ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_ENDING_RADIAN[animationID];

  float totalRotationDistance = endingRadian - startingRadian;
  float currentFrameRotation = startingRadian + (totalRotationDistance * easedInterpolationFactor);
  
  glm::vec2 pivot = ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_PIVOT_POINT[animationID];

  float radius = ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_RADIUS[animationID];

  float x = pivot.x + cosf(currentFrameRotation) * radius;
  float y = pivot.y + sinf(currentFrameRotation) * radius;

  auto gameObject = getGameObject(ANIMATION_SOA_VECTOR_TARGET_ID[animationID]);

  gameObject->transform2D.translation = {x, y};
  gameObject->transform2D.rotation = currentFrameRotation;
}

glm::vec2 LvsGameAnimations::vectorChangeCalculation(glm::vec2 &startingVector, glm::vec2 &endingVector, int animationID) {
  float animationTimeProgess = (
    ANIMATION_SOA_VECTOR_ANIMATION_ELAPSED_TIME[animationID] / ANIMATION_SOA_VECTOR_ANIMATION_DURATION[animationID]
  );
  if (animationTimeProgess > 1.0f) animationTimeProgess = 1.0f;

  if (ANIMATION_SOA_VECTOR_ANIMATION_REVERSE_ON_FINISH[animationID]) {
    animationTimeProgess = (animationTimeProgess <= 0.5f)
    ? (animationTimeProgess * 2.0f)
    : (2.0f - (animationTimeProgess * 2.0f));
  }

  float easedInterpolationFactor = (ANIMATION_SOA_VECTOR_ANIMATION_CUSTOM_EASE_FUNCTION[animationID] == nullptr)
  ? EASE_TABLE[ANIMATION_SOA_VECTOR_ANIMATION_EASE[animationID]](animationTimeProgess)
  : ANIMATION_SOA_VECTOR_ANIMATION_CUSTOM_EASE_FUNCTION[animationID](animationTimeProgess);

  if (ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_IS_RELATIVE[animationID]) {
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
    ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_STARTING_POSITION[animationID],
    ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_ENDING_POSITION[animationID],
    animationID
  );

  auto gameObject = getGameObject(ANIMATION_SOA_VECTOR_TARGET_ID[animationID]);

  gameObject->transform2D.translation.x = currentFramePosition.x;
  gameObject->transform2D.translation.y = currentFramePosition.y;
}

void LvsGameAnimations::setScalingAnimation(int animationID) {
  glm::vec2 currentFrameScale = vectorChangeCalculation(
    ANIMATION_SOA_VECTOR_SCALING_ANIMATION_STARTING_SCALE_VECTOR[animationID],
    ANIMATION_SOA_VECTOR_SCALING_ANIMATION_ENDING_SCALE_VECTOR[animationID],
    animationID
  );

  auto gameObject = getGameObject(ANIMATION_SOA_VECTOR_TARGET_ID[animationID]);

  gameObject->transform2D.scale.x = currentFrameScale.x;
  gameObject->transform2D.scale.y = currentFrameScale.y;
}

bool LvsGameAnimations::hasAnimation(int &animationID) {
  if (animationID < 0) return false;
  if (static_cast<size_t>(animationID) < ANIMATION_SOA_VECTOR_ANIMATION_NAME.size()) {
    return ANIMATION_SOA_VECTOR_ANIMATION_STATE[animationID] != AnimationState::ANIMATION_STATE_FINISHED;
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
#define SYNC_VAL(field, vector) if(writeToSOA) vector[idx] = field; else field = vector[idx];
#define SYNC_PUSH(field, vector) if(writeToSOA) vector.push_back(field);
  
  if (writeToSOA && (size_t)idx == ANIMATION_SOA_VECTOR_ANIMATION_TYPE.size()) {
    SYNC_PUSH(props.TYPE, ANIMATION_SOA_VECTOR_ANIMATION_TYPE);
    SYNC_PUSH(props.EASE, ANIMATION_SOA_VECTOR_ANIMATION_EASE);
    SYNC_PUSH(AnimationState::ANIMATION_STATE_STARTING, ANIMATION_SOA_VECTOR_ANIMATION_STATE);
    SYNC_PUSH(props.ANIMATION_NAME, ANIMATION_SOA_VECTOR_ANIMATION_NAME);
    SYNC_PUSH(props.TARGET_ID, ANIMATION_SOA_VECTOR_TARGET_ID);
    SYNC_PUSH(props.duration, ANIMATION_SOA_VECTOR_ANIMATION_DURATION);
    SYNC_PUSH(props.delay, ANIMATION_SOA_VECTOR_ANIMATION_DELAY);
    SYNC_PUSH(0.f, ANIMATION_SOA_VECTOR_ANIMATION_ELAPSED_TIME);
    SYNC_PUSH(0.f, ANIMATION_SOA_VECTOR_ANIMATION_ELAPSED_DELAY_TIME);
    SYNC_PUSH(props.repetition, ANIMATION_SOA_VECTOR_ANIMATION_REPETITION);
    SYNC_PUSH(false, ANIMATION_SOA_VECTOR_ANIMATION_DELAY_FINISHED);
    SYNC_PUSH(props.reverse_on_finish, ANIMATION_SOA_VECTOR_ANIMATION_REVERSE_ON_FINISH);
    SYNC_PUSH(props.CALLBACK_DATA, ANIMATION_SOA_VECTOR_ANIMATION_CALLBACK_DATA);
    SYNC_PUSH(props.ANIMATION_CUSTOM_EASE_FUNCTION, ANIMATION_SOA_VECTOR_ANIMATION_CUSTOM_EASE_FUNCTION);

    if (props.TYPE == ANIMATION_TYPE_ROTATION) {
      SYNC_PUSH(props.rotation.starting_radian, ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_STARTING_RADIAN);
      SYNC_PUSH(props.rotation.pivot_point, ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_PIVOT_POINT);
      SYNC_PUSH(props.rotation.ending_radian, ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_ENDING_RADIAN);
      SYNC_PUSH(props.rotation.radius, ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_RADIUS);
    } else {
      SYNC_PUSH(0.f, ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_STARTING_RADIAN);
      SYNC_PUSH(glm::vec2(0.f), ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_PIVOT_POINT);
      SYNC_PUSH(0.f, ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_ENDING_RADIAN);
      SYNC_PUSH(1.f, ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_RADIUS);
    }

    if (props.TYPE == ANIMATION_TYPE_TRANSLATION) {
      SYNC_PUSH(props.translation.starting_position, ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_STARTING_POSITION);
      SYNC_PUSH(props.translation.ending_position, ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_ENDING_POSITION);
      SYNC_PUSH(props.translation.is_relative, ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_IS_RELATIVE);
    } else {
      SYNC_PUSH(glm::vec2(0.f), ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_STARTING_POSITION);
      SYNC_PUSH(glm::vec2(0.f), ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_ENDING_POSITION);
      SYNC_PUSH(false, ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_IS_RELATIVE);
    }

    if (props.TYPE == ANIMATION_TYPE_SCALE) {
      SYNC_PUSH(props.scale.starting_scale, ANIMATION_SOA_VECTOR_SCALING_ANIMATION_STARTING_SCALE_VECTOR);
      SYNC_PUSH(props.scale.ending_scale, ANIMATION_SOA_VECTOR_SCALING_ANIMATION_ENDING_SCALE_VECTOR);
    } else {
      SYNC_PUSH(glm::vec2(1.f), ANIMATION_SOA_VECTOR_SCALING_ANIMATION_STARTING_SCALE_VECTOR);
      SYNC_PUSH(glm::vec2(1.f), ANIMATION_SOA_VECTOR_SCALING_ANIMATION_ENDING_SCALE_VECTOR);
    }
  } else {
    SYNC_VAL(props.TYPE, ANIMATION_SOA_VECTOR_ANIMATION_TYPE);
    SYNC_VAL(props.EASE, ANIMATION_SOA_VECTOR_ANIMATION_EASE);
    SYNC_VAL(props.ANIMATION_STATE, ANIMATION_SOA_VECTOR_ANIMATION_STATE);
    SYNC_VAL(props.ANIMATION_NAME, ANIMATION_SOA_VECTOR_ANIMATION_NAME);
    SYNC_VAL(props.TARGET_ID, ANIMATION_SOA_VECTOR_TARGET_ID);
    SYNC_VAL(props.duration, ANIMATION_SOA_VECTOR_ANIMATION_DURATION);
    SYNC_VAL(props.delay, ANIMATION_SOA_VECTOR_ANIMATION_DELAY);
    SYNC_VAL(props.elapsed_time, ANIMATION_SOA_VECTOR_ANIMATION_ELAPSED_TIME);
    SYNC_VAL(props.elapsed_delay_time, ANIMATION_SOA_VECTOR_ANIMATION_ELAPSED_DELAY_TIME);
    SYNC_VAL(props.repetition, ANIMATION_SOA_VECTOR_ANIMATION_REPETITION);
    SYNC_VAL(props.delay_finished, ANIMATION_SOA_VECTOR_ANIMATION_DELAY_FINISHED);
    SYNC_VAL(props.reverse_on_finish, ANIMATION_SOA_VECTOR_ANIMATION_REVERSE_ON_FINISH);
    SYNC_VAL(props.CALLBACK_DATA, ANIMATION_SOA_VECTOR_ANIMATION_CALLBACK_DATA);
    SYNC_VAL(props.ANIMATION_CUSTOM_EASE_FUNCTION, ANIMATION_SOA_VECTOR_ANIMATION_CUSTOM_EASE_FUNCTION);

    if (writeToSOA) {
      if (props.TYPE == ANIMATION_TYPE_ROTATION) {
        ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_STARTING_RADIAN[idx] = props.rotation.starting_radian;
        ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_PIVOT_POINT[idx] = props.rotation.pivot_point;
        ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_ENDING_RADIAN[idx] = props.rotation.ending_radian;
        ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_RADIUS[idx] = props.rotation.radius;
      } else if (props.TYPE == ANIMATION_TYPE_TRANSLATION) {
        ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_STARTING_POSITION[idx] = props.translation.starting_position;
        ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_ENDING_POSITION[idx] = props.translation.ending_position;
        ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_IS_RELATIVE[idx] = props.translation.is_relative;
      } else if (props.TYPE == ANIMATION_TYPE_SCALE) {
        ANIMATION_SOA_VECTOR_SCALING_ANIMATION_STARTING_SCALE_VECTOR[idx] = props.scale.starting_scale;
        ANIMATION_SOA_VECTOR_SCALING_ANIMATION_ENDING_SCALE_VECTOR[idx] = props.scale.ending_scale;
      }
    } else {
      props.rotation.starting_radian = ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_STARTING_RADIAN[idx];
      props.rotation.pivot_point = ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_PIVOT_POINT[idx];
      props.rotation.ending_radian = ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_ENDING_RADIAN[idx];
      props.rotation.radius = ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_RADIUS[idx];
      props.translation.starting_position = ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_STARTING_POSITION[idx];
      props.translation.ending_position = ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_ENDING_POSITION[idx];
      props.translation.is_relative = ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_IS_RELATIVE[idx];
      props.scale.starting_scale = ANIMATION_SOA_VECTOR_SCALING_ANIMATION_STARTING_SCALE_VECTOR[idx];
      props.scale.ending_scale = ANIMATION_SOA_VECTOR_SCALING_ANIMATION_ENDING_SCALE_VECTOR[idx];
    }
  }
#undef SYNC_VAL
#undef SYNC_PUSH
}

/**
 * @brief a function which initializes a new animation | linking the animationProperties with the gameObject
 * 
 * @bug Empty/Whitespace Name: Runtime error occurs if animation name is empty or only whitespace. Fix: Validate that names contain non-whitespace characters.
 * @bug Duplicate Name Collision: Renaming an animation to an existing name generates a warning and triggers unintended auto-renaming (e.g., AnimName(x)). Fix: Prevent name collisions and handle renames properly.
 * @bug Negative Duration/Delay: Making Duration or Delay negative will throw a runtime error, Fix: Make sure to set Duration or Delay higher or equal to 0
 * 
 * @param animationProperties A structure which contains all of the animation information
 * @param gameObject the object that is going to be animated
 */
int LvsGameAnimations::setAnimation(AnimationProperties &animationProperties) {
  std::cout << cpc::Cyan << "Creating Animation..." << cpc::Reset << std::endl;

  bool isOnlyWhitespace = std::all_of(animationProperties.ANIMATION_NAME.begin(), animationProperties.ANIMATION_NAME.end(), [](unsigned char ch) { return std::isspace(ch); });
  if (animationProperties.ANIMATION_NAME.empty() || isOnlyWhitespace) {
    throw std::runtime_error("RUNTIME ERROR: Animation name cannot be empty or contain only spaces!");
  }

  int collisionCount = 0;
  for (const auto& existingAnim : activeAnimations) {
    if (existingAnim.ANIMATION_NAME.find(animationProperties.ANIMATION_NAME)) {
      collisionCount++;
    }
  }

  if (collisionCount > 0) {
    std::string oldName = animationProperties.ANIMATION_NAME;
    animationProperties.ANIMATION_NAME += std::to_string(collisionCount);

    std::cout << cpc::Yellow << "WARNING: Animation name of: {" << oldName << "} matches with another animation name | " << 
      "New Animation name setted to: {" << animationProperties.ANIMATION_NAME << "}" << cpc::Reset << std::endl;
  }

  if (animationProperties.duration < 0) { std::runtime_error("Animation Duration can't be less then zero"); }
  if (animationProperties.delay < 0) { std::runtime_error("Animation Delay can't be less than zero"); }

  animationProperties.delay_finished = (animationProperties.delay <= animationProperties.elapsed_delay_time);

  int idxToInsertIn;
  bool recycling = false;

  if (!ANIMATION_SOA_VECTOR_ANIMATION_FREE_SLOTS.empty()) {
    idxToInsertIn = ANIMATION_SOA_VECTOR_ANIMATION_FREE_SLOTS.back();
    ANIMATION_SOA_VECTOR_ANIMATION_FREE_SLOTS.pop_back();
    recycling = true;
  } else {
    idxToInsertIn = static_cast<int>(ANIMATION_SOA_VECTOR_ANIMATION_TYPE.size());
  }

  if (recycling) {
    syncPropertiesWithSOA(idxToInsertIn, animationProperties, true);
  } else {
    syncPropertiesWithSOA(idxToInsertIn, animationProperties, true);
  }

  ANIMATION_SOA_VECTOR_ANIMATION_ACTIVE_INDICES.push_back(idxToInsertIn);

  std::cout << cpc::Green << "Successfully created animation with animation name: " << animationProperties.ANIMATION_NAME << cpc::Reset << std::endl;
  activeAnimations.push_back(std::move(animationProperties));

  return idxToInsertIn;
}

void LvsGameAnimations::checkArgumentValidity(int animationID, AnimationState animationState) {
  if (hasAnimation(animationID)) {
    if (ANIMATION_SOA_VECTOR_ANIMATION_STATE[animationID] == animationState) return;
    ANIMATION_SOA_VECTOR_ANIMATION_STATE[animationID] = animationState;
  } else {
    std::cout << cpc::Yellow << "Did not find a animation that matches with id: " 
              << animationID << cpc::Reset << std::endl;
  }
}

/**
 * @brief Pauses the animation from moving and changes the state to ANIMATION_STATE_PAUSED
 * 
 * @note if animation name is not provided game object argument can be used HOWEVER both animation name and game object can't be null pointer
 * 
 * @bug Null pointer arguments: If both animationName and gameObject are null pointers the function will throw a runtime error. FIX: set atleast 1 of the arguments to be a valid argument
 * 
 * @param animationName a pointer to the animation name can be nullpointer if not provided but game object is provided
 * @param gameObject  a pointer to the game object can be nullpointer if not provided but animation name is provided
 */
void LvsGameAnimations::pauseAnimation(int animationID) {
  checkArgumentValidity(animationID, ANIMATION_STATE_PAUSED);
}

/**
 * @brief Continues the animation if paused and changes the animation state to ANIMATION_STATE_DELAYED which will automatically change to ANIMATION_STATE_PLAYING if not delayed
 * 
 * @note if animation name is not provided game object argument can be used HOWEVER both animation name and game object can't be null pointer
 * 
 * @bug Null pointer arguments: If both animationName and gameObject are null pointers the function will throw a runtime error. FIX: set atleast 1 of the arguments to be a valid argument
 * 
 * @param animationName a pointer to the animation name can be nullpointer if not provided but game object is provided
 * @param gameObject  a pointer to the game object can be nullpointer if not provided but animation name is provided
 */
void LvsGameAnimations::continueAnimation(int animationID) {
  checkArgumentValidity(animationID, ANIMATION_STATE_DELAYING); // Will automatically set to ANIMATON_STATE_PLAYING if finished delay
}

void LvsGameAnimations::resetToOriginalState(int &animationID) {
  auto gameObject = getGameObject(ANIMATION_SOA_VECTOR_TARGET_ID[animationID]);

  switch(ANIMATION_SOA_VECTOR_ANIMATION_TYPE[animationID]) {
  case ANIMATION_TYPE_ROTATION:
    gameObject->transform2D.rotation = ANIMATION_SOA_VECTOR_ROTATION_ANIMATION_STARTING_RADIAN[animationID];
    break;
  case ANIMATION_TYPE_SCALE:
    gameObject->transform2D.scale = ANIMATION_SOA_VECTOR_SCALING_ANIMATION_STARTING_SCALE_VECTOR[animationID];
    break;
  case ANIMATION_TYPE_TRANSLATION:
    gameObject->transform2D.translation = ANIMATION_SOA_VECTOR_TRANSLATION_ANIMATION_STARTING_POSITION[animationID];
    break;
  }
}

bool LvsGameAnimations::checkAnimationState(int animationID)  {
  AnimationState& animationState   = ANIMATION_SOA_VECTOR_ANIMATION_STATE[animationID];
  float& animationElapsedDelayTime = ANIMATION_SOA_VECTOR_ANIMATION_ELAPSED_DELAY_TIME[animationID];
  float& animationElapsedTime      = ANIMATION_SOA_VECTOR_ANIMATION_ELAPSED_TIME[animationID];
  float& animationDuration         = ANIMATION_SOA_VECTOR_ANIMATION_DURATION[animationID];
  float& animationDelay            = ANIMATION_SOA_VECTOR_ANIMATION_DELAY[animationID];
  int& animationRepetition         = ANIMATION_SOA_VECTOR_ANIMATION_REPETITION[animationID];
  bool animationDelayFinished      = ANIMATION_SOA_VECTOR_ANIMATION_DELAY_FINISHED[animationID];
  

  if (animationState == ANIMATION_STATE_PAUSED) { 
    return true;
  }

  if (!animationDelayFinished) {
    if (animationState != ANIMATION_STATE_DELAYING) animationState = ANIMATION_STATE_DELAYING;

    animationElapsedDelayTime += Simulation::FrameInformation.deltaFrameTime;

    ANIMATION_SOA_VECTOR_ANIMATION_DELAY_FINISHED[animationID] = (
      animationDelay <= animationElapsedDelayTime
    );
    if (!animationDelayFinished) {
      return true;
    }

    animationElapsedDelayTime = (
      animationElapsedDelayTime - animationDelay
    );
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
      return true; // Wait 1 frame before deleting the animation
    }

    std::cout << cpc::Cyan << "Cleaning up finished animaiton: " << ANIMATION_SOA_VECTOR_ANIMATION_NAME[animationID] << cpc::Reset << std::endl;
    
    std::vector<int>& activeIndicies = ANIMATION_SOA_VECTOR_ANIMATION_ACTIVE_INDICES;
    int foundIndexInActive = -1;
    for (int i = 0; i < activeIndicies.size(); i++) {
      if (activeIndicies[i] == animationID) {
        foundIndexInActive = i;
        break;
      }
    }

    if (foundIndexInActive != -1) {
      activeIndicies[foundIndexInActive] = activeIndicies.back();
      activeIndicies.pop_back();

      animationState = ANIMATION_STATE_DELETED;
      ANIMATION_SOA_VECTOR_ANIMATION_FREE_SLOTS.push_back(animationID);

      std::cout << cpc::Cyan << "Recycled Slot: " << animationID << ". Active count: " << activeIndicies.size() << cpc::Reset << std::endl;
    }
    return true;
  }

  return false;
}

/**
 * @brief Update animation is called before the frame is about to be drawn, and updates all current animations EXCEPT for paused animations
 * 
 * @details first the function will check the state of the function if the state of the function is 
 * 1- Delayed it will increase the ANIMATION_ELAPSED_DELAY_TIME till it is more than or equal to the ANIMATION_DELAY property once that happens the ANIMATION_DELAY_FINISHED will become true and the ANIMATION_STATE will change
 * 2- It will check if the animation is finished if it is it will wait 1 frame before destroying the animationProperty
 * 3- if not delayed or finished it will continue onto playing the animation and ANIMATION_STATE will be set to ANIMATION_STATE_PLAYING
 */
void LvsGameAnimations::updateAnimations(std::unordered_map<LvsGameObject::id_t, LvsGameObject>& world) {
  GAME_OBJECTS = &world;

  for (int idx : ANIMATION_SOA_VECTOR_ANIMATION_ACTIVE_INDICES) {
    if (checkAnimationState(idx)) continue;

    switch (ANIMATION_SOA_VECTOR_ANIMATION_TYPE[idx]) {
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