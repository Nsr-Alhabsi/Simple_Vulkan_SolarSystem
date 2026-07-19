#pragma once

#include "Core/lvs_morph.hpp"
#include "../../models/lvs_model.hpp"

namespace lvs {

class LvsMorphCalculation {
public:
  std::vector<LvsModel::Vertex> calculateMorph(
    const std::vector<std::vector<LvsModel::Vertex>> &verticesList, const LvsMorph::morphProperties &props, float deltaTime
  );
};

}