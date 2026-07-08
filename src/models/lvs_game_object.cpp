#include "lvs_game_object.hpp"
#include "../ADDONS/cp_color.hpp"

// std:
#include <stdexcept>
#include <unordered_map>

namespace lvs {

static std::shared_ptr<LvsModel> circleModel = nullptr;
static std::shared_ptr<LvsModel> triangleModel = nullptr;
static std::shared_ptr<LvsModel> squareModel = nullptr;
static std::unordered_map<LvsGameObject::id_t, int> s_ObjectTypeMap;

std::vector<LvsModel::Vertex> LvsGameObject::createObjectVertices(int typeOfObject) {
  std::vector<LvsModel::Vertex> vertices;

  switch (typeOfObject) {
    case 0: { 
      float radius = 1.0f;
      int count = 128;
      vertices.reserve(static_cast<size_t>(count) * 3);
      
      const float angleStep = glm::two_pi<float>() / static_cast<float>(count);

      for (int i = 0; i < count; i++) {
        float a1 = static_cast<float>(i) * angleStep;
        float a2 = static_cast<float>(i + 1) * angleStep;

        // Positions
        glm::vec2 p0 = {0.0f, 0.0f};
        glm::vec2 p1 = {radius * glm::cos(a1), radius * glm::sin(a1)};
        glm::vec2 p2 = {radius * glm::cos(a2), radius * glm::sin(a2)};

        auto getUV = [&](glm::vec2 p) { return p * 0.5f + 0.5f; };

        vertices.push_back({p0, {1.f, 1.f, 1.f}, getUV(p0)});
        vertices.push_back({p2, {1.f, 1.f, 1.f}, getUV(p2)});
        vertices.push_back({p1, {1.f, 1.f, 1.f}, getUV(p1)});
      }
      break;
    }
    case 1: { // Triangle
      vertices.reserve(3);
      float h2 = 0.5f;
      float w2 = 0.5f;
      // Format: {position, color, uv}
      vertices.push_back({{0.0f, -h2}, {1,1,1}, {0.5f, 0.0f}});
      vertices.push_back({{-w2, h2},  {1,1,1}, {0.0f, 1.0f}});
      vertices.push_back({{w2, h2},   {1,1,1}, {1.0f, 1.0f}});
      break;
    }
    case 2: { // Square
      vertices.reserve(6);
      float h2 = 0.5f;
      float w2 = 0.5f;
      glm::vec2 tl = {-w2, -h2};
      glm::vec2 tr = {w2, -h2};
      glm::vec2 bl = {-w2, h2};
      glm::vec2 br = {w2, h2};
      
      // UVs: tl{0,0}, tr{1,0}, bl{0,1}, br{1,1}
      vertices.push_back({tl, {1,1,1}, {0.f, 0.f}});
      vertices.push_back({tr, {1,1,1}, {1.f, 0.f}});
      vertices.push_back({bl, {1,1,1}, {0.f, 1.f}});
      vertices.push_back({tr, {1,1,1}, {1.f, 0.f}});
      vertices.push_back({br, {1,1,1}, {1.f, 1.f}});
      vertices.push_back({bl, {1,1,1}, {0.f, 1.f}});
      break;
    }
  }

  return vertices;
}

LvsGameObject LvsGameObject::createGameObject(int typeOfObject, LvsDevice& device, std::vector<LvsModel::Vertex> *customVertices) {
  static id_t currentId = 0;

  if (typeOfObject == ObjectType::Custom) {
    if (!customVertices || customVertices->empty() || customVertices->size() % 3 != 0) {
      throw std::runtime_error(cpc::Red + "Cannot create custom game object: customVertices must be non-null, "
                                "non-empty, and a multiple of 3 (triangle list topology)." + cpc::Reset);
    }

    auto customModel = std::make_shared<LvsModel>(device, *customVertices);
    LvsGameObject obj{currentId++};
    obj.model = customModel;
    s_ObjectTypeMap[obj.id] = ObjectType::Custom;
    return obj;
  }

  if (typeOfObject == ObjectType::Circle) {
    if (!circleModel) {
      auto vertices = LvsGameObject::createObjectVertices(0);
      circleModel = std::make_shared<LvsModel>(device, vertices);
    }
    LvsGameObject obj{currentId++};
    obj.model = circleModel;
    s_ObjectTypeMap[obj.id] = 0;
    return obj;
  }

  if (typeOfObject == ObjectType::Triangle) {
    if (!triangleModel) {
      auto vertices = LvsGameObject::createObjectVertices(1);
      triangleModel = std::make_shared<LvsModel>(device, vertices);
    }
    LvsGameObject obj{currentId++};
    obj.model = triangleModel;
    s_ObjectTypeMap[obj.id] = 1;
    return obj;
  }

  if (typeOfObject == ObjectType::Square) {
    if (!squareModel) {
      auto vertices = LvsGameObject::createObjectVertices(2);
      squareModel = std::make_shared<LvsModel>(device, vertices);
    }
    LvsGameObject obj{currentId++};
    obj.model = squareModel;
    s_ObjectTypeMap[obj.id] = 2;
    return obj;
  }

  throw std::invalid_argument("Invalid object type");
}

std::vector<LvsModel::Vertex> LvsGameObject::getObjectVertices(id_t id) {
  auto it = s_ObjectTypeMap.find(id);
  if (it == s_ObjectTypeMap.end()) return {};
  return createObjectVertices(it->second);
}

glm::mat3 LvsGameObject::getGlobalMatrix(std::unordered_map<id_t, LvsGameObject>& objectList) {
  glm::mat3 worldMatrix = transform2D.mat3();

  if (hasParent) {
    auto it = objectList.find(parentId);
    if (it != objectList.end()) {
      worldMatrix = it->second.getGlobalMatrix(objectList) * worldMatrix;
    }
  }
  
  return worldMatrix;
}

}