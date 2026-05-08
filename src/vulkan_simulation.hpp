#pragma once

#include "core/window_setup.hpp"
#include "models/lvs_game_object.hpp"
#include "models/lvs_renderer.hpp"
#include "models/lvs_game_animations.hpp"

#include <nlohmann/json.hpp>
#include <glm/gtc/constants.hpp>

#include <chrono>
#include <memory>
#include <vector>

namespace lvs {

struct FrameInfo {
  float currentFPS{0.f};
  float avgFrameTimeMs{0.f}; // Total time / frames
  float minFrameTimeMs{1.f}; // Fastest frame
  float maxFrameTimeMs{0.f}; // Slowest frame | "Stutter" indicator
  float deltaFrameTime{0.f};
  uint64_t totalTime{0};
  uint64_t totalFrames{0}; 
};

class Simulation {

struct ObjectType {
  enum {
    Circle = 0,
    Triangle = 1,
    Square = 2
  };
};

public:
  Simulation();
  ~Simulation();

  Simulation(const Simulation &) = delete;
  Simulation& operator=(const Simulation &) = delete;

  static inline FrameInfo FrameInformation{};

  /**
   * @brief main function which diverges into multiple functions which control the flow of the simulation.
   * 
   */
  void run();
private:
  void loadGameObjects();
  void createCircleVertices(
    std::vector<LvsModel::Vertex> &vertices,
    int Smoothness, float radius,
    const glm::vec3 &centerColor,
    const glm::vec3 &edgeColor
  );
  void updateFPS(std::chrono::high_resolution_clock::time_point &lastTime);

  LvsWindow lvsWindow; // empty since i already specified the window height, width and title in the AppSettings.json file.
  LvsDevice lvsDevice{lvsWindow};
  LvsRenderer lvsRenderer{lvsWindow, lvsDevice};

  std::vector<LvsGameObject> gameObjects;
};

}