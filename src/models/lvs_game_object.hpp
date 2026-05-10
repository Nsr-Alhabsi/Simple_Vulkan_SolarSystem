#pragma once

#include "lvs_model.hpp"
#include "lvs_game_animations.hpp"

// libs:
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/constants.hpp>

//std:
#include <memory>

namespace lvs {

struct Transform2DComponent {
  glm::vec2 translation{}; // (position offset)
  glm::vec2 scale{1.f, 1.f};
  float rotation;

  glm::mat2 mat2() {
    const float s = glm::sin(rotation);
    const float c = glm::cos(rotation);
    glm::mat2 rotMatrix{{c, s}, {-s, c}};

    glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
    return rotMatrix * scaleMat;
  }
};

class LvsGameObject {
public:
  using id_t = unsigned int;

  static LvsGameObject createGameObject(int typeOfObject, LvsDevice& device);

  LvsGameObject(const LvsGameObject &) = delete;
  LvsGameObject &operator=(const LvsGameObject &) = delete;
  LvsGameObject(LvsGameObject&&) = default;
  LvsGameObject &operator=(LvsGameObject&&) = default;

  id_t getId() {return id;}

  std::shared_ptr<LvsModel> model{};
  glm::vec3 color{}; // This will only work if isGradient is false
  glm::vec3 color2{};
  glm::vec2 gradDir{};
  bool isGradient{false};

  Transform2DComponent transform2D{};
  private:
  static std::vector<LvsModel::Vertex> createObjectVertices(int typeOfObject);
  LvsGameObject(id_t objId) : id{objId} {}
  id_t id;
};
}