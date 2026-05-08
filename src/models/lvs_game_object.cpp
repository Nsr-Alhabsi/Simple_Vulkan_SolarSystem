#include "lvs_game_object.hpp"

// std:
#include <unordered_map>

namespace lvs {

std::vector<LvsModel::Vertex> LvsGameObject::createObjectVertices(int typeOfObject) {
  std::vector<LvsModel::Vertex> vertices;

  switch (typeOfObject) {
    case 0: {
      float radius = 1.0f;
      float radianRadius = glm::two_pi<float>();
      int count = 60;
      vertices.reserve(count + 2);
      vertices.push_back({{0.0f, 0.0f}});
      for (int i = 0; i <= count; i++) {
        float angle = (static_cast<float>(i) / count) * radianRadius;
        float x = radius * glm::cos(angle);
        float y = radius * glm::sin(angle);
        vertices.push_back({{x, y}});
      }
      break;
    }
    case 1: {
      vertices.reserve(3);
      float h2 = 0.5f;
      float w2 = 0.5f;
      vertices.push_back({{0.0f, -h2}});
      vertices.push_back({{-w2, h2}});
      vertices.push_back({{w2, h2}});
      break;
    }
    case 2: {
      vertices.reserve(6);
      float h2 = 0.5f;
      float w2 = 0.5f;
      glm::vec2 tl = {-w2, -h2};
      glm::vec2 tr = {w2, -h2};
      glm::vec2 bl = {-w2, h2};
      glm::vec2 br = {w2, h2};
      vertices.push_back({tl});
      vertices.push_back({tr});
      vertices.push_back({bl});
      vertices.push_back({tr});
      vertices.push_back({br});
      vertices.push_back({bl});
      break;
    }
  }

  return vertices;
}

static std::shared_ptr<LvsModel> circleModel = nullptr;
static std::shared_ptr<LvsModel> triangleModel = nullptr;
static std::shared_ptr<LvsModel> squareModel = nullptr;

LvsGameObject LvsGameObject::createGameObject(int typeOfObject, LvsDevice& device) {
  static id_t currentId = 0;

  if (typeOfObject == 0) {
    if (!circleModel) {
      auto vertices = LvsGameObject::createObjectVertices(0);
      circleModel = std::make_shared<LvsModel>(device, vertices);
    }
    LvsGameObject obj{currentId++};
    obj.model = circleModel;
    return obj;
  }

  if (typeOfObject == 1) {
    if (!triangleModel) {
      auto vertices = LvsGameObject::createObjectVertices(1);
      triangleModel = std::make_shared<LvsModel>(device, vertices);
    }
    LvsGameObject obj{currentId++};
    obj.model = triangleModel;
    return obj;
  }

  if (!squareModel) {
    auto vertices = LvsGameObject::createObjectVertices(2);
    squareModel = std::make_shared<LvsModel>(device, vertices);
  }
  LvsGameObject obj{currentId++};
  obj.model = squareModel;
  return obj;
}

}