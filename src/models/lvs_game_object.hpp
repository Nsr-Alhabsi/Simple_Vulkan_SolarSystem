#pragma once

#include "lvs_model.hpp"
#include "properties/lvs_game_animations.hpp"

// libs:
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/constants.hpp>

//std:
#include <memory>
#include <unordered_map>

namespace lvs {

struct Transform2DComponent {
  glm::vec2 translation{}; // (position offset)
  glm::vec2 scale{1.f, 1.f};
  float rotation;

  glm::mat3 mat3() {
    const float c = cos(rotation);
    const float s = sin(rotation);

    return glm::mat3 {
      {scale.x * c, scale.x * s, 0.0f},
      {scale.y * -s, scale.y * c, 0.0f},
      {translation.x, translation.y, 1.0f}
    };
  }
};

class LvsGameObject {
public:
  struct ObjectType {
    enum {
      Custom = -1,
      Circle = 0,
      Triangle = 1,
      Square = 2
    };
  };

  using id_t = unsigned int;

  LvsGameObject() : id{0} {}
  static LvsGameObject createGameObject(int typeOfObject, LvsDevice& device, std::vector<LvsModel::Vertex> *customVertices = nullptr);
  static std::vector<LvsModel::Vertex> getObjectVertices(id_t id);

  glm::mat3 getGlobalMatrix(std::unordered_map<unsigned int, LvsGameObject>& objectList);

  LvsGameObject(const LvsGameObject &) = delete;
  LvsGameObject &operator=(const LvsGameObject &) = delete;
  LvsGameObject(LvsGameObject&&) = default;
  LvsGameObject &operator=(LvsGameObject&&) = default;

  id_t getId() {return id;}

  std::shared_ptr<LvsModel> model{};
  glm::vec3 color{};
  glm::vec3 color2{}; // This will only work if isGradient is true
  glm::vec2 gradDir{};
  bool isGradient{false};
  bool visible{true};

  id_t parentId = 0;
  bool hasParent = false;

  Transform2DComponent transform2D{};
  private:
  static std::vector<LvsModel::Vertex> createObjectVertices(int typeOfObject);
  LvsGameObject(id_t objId) : id{objId} {}
  id_t id;
};
}