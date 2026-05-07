#pragma once

#include <iostream>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace lvs { 

class LvsWindow {
public:
  /**
   * @brief Constructs a window for the Vulkan simulation using GLFW. It initializes the window with specified height, width, and title, which are retrieved from a JSON configuration file. If the window initialization fails, it throws a runtime error with an appropriate message.
   * 
   * @note The window's height, width, and title can be configured in the AppSettings.json file under the "Display_Settings" section. Specifically:
   * - Height: AppSettings.json -> Display_Settings -> Screen -> Height
   * - Width: AppSettings.json -> Display_Settings -> Screen -> Width
   * - Title: AppSettings.json -> Display_Settings -> Window_Title
   * 
   * @bug If the window initialization fails, the destructor will be called to clean up any resources that may have been allocated before the failure was detected. This ensures that even in the event of an initialization failure, resources are properly released, preventing potential memory leaks or other issues.
   */
  LvsWindow();

  /**
   * @brief Destroys the window and terminates GLFW to clean up resources.
   * 
   * @note This destructor is called when the LvsWindow object goes out of scope or is explicitly deleted. It ensures that any resources allocated for the window are properly released, preventing memory leaks and ensuring a clean shutdown of the application.
   */
  ~LvsWindow();

  /**
   * @brief returns whether the application should end or continue
   * 
   * @return true --> The 'X' has been pressed, meaning that the window should close 
   * @return false  --> The 'X' has not been pressed, causeing the simulation to continue
   */
  bool shouldClose() { return glfwWindowShouldClose(m_simulation_window); }
  VkExtent2D getExtent() { return { static_cast<uint32_t>(m_window_width), static_cast<uint32_t>(m_window_height) }; }
  bool wasWindowResized() { return frameBufferResized; };
  void resetWidnowResizedFlag() { frameBufferResized = true; };

  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
private:
  GLFWwindow* m_simulation_window;

  int m_window_height;
  int m_window_width;
  bool frameBufferResized = false;

  const std::string m_window_title;
  
  static void frameBufferResizedCallback(GLFWwindow *window, int width, int height);

  /**
   * @brief initlizes the window with a specific height, width and windowtitle
   * 
   * @param m_window_height
   * - can be changed in the AppSettings.json file in -> Display_Settings -> Screen -> Width
   * @param m_window_width
   * - can be changed in the AppSettings.json file in -> Display_Settings -> Screen -> Height
   * @param m_window_title
   * - can be changed in the AppSettings.json file in -> Display_Settings -> Window_Title
   * 
   * @return can either return 1 or 0 
   * - if reutrned 0 this means the function was successful
   * - if returned 1 this means the function had a issue and was stopped. 
   * 
   * @bug if the function fails to initalize the window, the destructor will be called to clean up any resources that may have been allocated before the failure was detected. This ensures that even in the event of an initialization failure, resources are properly released, preventing potential memory leaks or other issues.
   * 
   * @note the window will start at the main monitor, since nullptr was set as the starting monitor which defaults to the main.
   */
  int initWindow();
};

}