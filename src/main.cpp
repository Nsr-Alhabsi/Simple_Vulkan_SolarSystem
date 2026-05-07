#include "vulkan_simulation.hpp"
#include "ADDONS/cp_color.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
  std::cout << cpc::White << "Starting Vulkan Simulation..." << cpc::Reset << std::endl;
  lvs::Simulation app{};

  try {
    std::cout << cpc::White << "Starting Vulkan Simulation Run Loop..." << cpc::Reset << std::endl;
    app.run();
  } catch(const std::exception &e) {
    std::cerr << cpc::Red << "Error running application: " << e.what() << cpc::Reset << std::endl;
    return EXIT_FAILURE;
  }
  
  std::cout << cpc::White << "Ending Vulkan Simulation..." << cpc::Reset << std::endl;
  return EXIT_SUCCESS;
}