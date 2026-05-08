#include "lvs_game_object.hpp"

namespace lvs {

std::vector<LvsModel::Vertex> LvsGameObject::createObjectVertices(ObjectType typeOfObject) {
  std::vector<LvsModel::Vertex> vertices;

  if (typeOfObject.Circle.smoothness > 0) {
    int count = typeOfObject.Circle.smoothness;
    vertices.reserve(count + 2);

    vertices.push_back({{0.0f, 0.0f}});

    for (int i = 0; i <= count; i++) {
      float angle = (static_cast<float>(i) / count) * typeOfObject.Circle.displayRadianRadius;
      float x = typeOfObject.Circle.radius * glm::cos(angle);
      float y = typeOfObject.Circle.radius * glm::sin(angle);
      vertices.push_back({{x, y}});
    }
  } 
  else if (typeOfObject.Triangle.height != 0) {
    vertices.reserve(3);
    float h2 = typeOfObject.Triangle.height / 2.0f;
    float w2 = typeOfObject.Triangle.width / 2.0f;
    float lean = typeOfObject.Triangle.lean;

    vertices.push_back({{0.0f + lean, -h2}});
    vertices.push_back({{-w2, h2}});
    vertices.push_back({{w2, h2}});
  }
  else {
    vertices.reserve(6);
    float h2 = typeOfObject.Square.height / 2.0f;
    float w2 = typeOfObject.Square.width / 2.0f;
    float skew = typeOfObject.Square.skew;

    glm::vec2 tl = {-w2 + skew, -h2};
    glm::vec2 tr = {w2 + skew, -h2};
    glm::vec2 bl = {-w2, h2};
    glm::vec2 br = {w2, h2};

    vertices.push_back({tl});
    vertices.push_back({tr});
    vertices.push_back({bl});
    vertices.push_back({tr});
    vertices.push_back({br});
    vertices.push_back({bl});
  }

  return vertices;
}

LvsGameObject LvsGameObject::createGameObject(ObjectType typeOfObject) {
  static id_t currentId = 0;

  LvsGameObject  obj{currentId++};
  return obj;
}

}