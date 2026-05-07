#define GLFW_INCLUDE_VULKAN

#include "../ADDONS/json_setup.hpp"
#include "../ADDONS/cp_color.hpp"
#include "window_setup.hpp"

#include <iostream>
#include <exception>

#include <nlohmann/json.hpp>
#include <GLFW/glfw3.h>

using json = nlohmann::json;

namespace lvs { // lvs stands for large - vulkan - simulation

namespace {
  json displaySettings = jsf::getSettings(jsf::DISPLAY_SETTINGS);
}

LvsWindow::LvsWindow() 
  : m_window_height(displaySettings["Screen"]["Height"]), m_window_width(displaySettings["Screen"]["Width"]),
    m_window_title(displaySettings["Window_Title"])
{
  std::cerr << cpc::White << "DEBUG JSON: " << jsf::getSettings(jsf::SETTINGS).dump(4) << cpc::Reset << std::endl;
  if(initWindow() != EXIT_SUCCESS) {
    throw std::runtime_error(cpc::Red + "Failed to initalize the window!" + cpc::Reset);
    LvsWindow::~LvsWindow();
  }
  std::cout << cpc::Green << "Window initalized successfully!" << cpc::Reset << std::endl;
}

LvsWindow::~LvsWindow() {
  glfwDestroyWindow(m_simulation_window);
  glfwTerminate();
}

int LvsWindow::initWindow() {
  if (!glfwInit()) {
    throw std::runtime_error(cpc::Red + "Failed to initalize GLFW!" + cpc::Reset);
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  m_simulation_window = glfwCreateWindow(m_window_width, m_window_height, m_window_title.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(m_simulation_window, this);
  glfwSetFramebufferSizeCallback(m_simulation_window, frameBufferResizedCallback);
  return EXIT_SUCCESS;
}

void LvsWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, m_simulation_window, nullptr, surface) != VK_SUCCESS) {
    throw std::runtime_error(cpc::Red + "Failed to create window surface!" + cpc::Reset);
  }
  std::cout << cpc::Green << "Window surface created successfully!" << cpc::Reset << std::endl;
}

void LvsWindow::frameBufferResizedCallback(GLFWwindow *window, int width, int height) {
  auto lvsWindow = reinterpret_cast<LvsWindow *>(glfwGetWindowUserPointer(window));
  lvsWindow->frameBufferResized = true;
  lvsWindow->m_window_width = width;
  lvsWindow->m_window_height = height;
}

}