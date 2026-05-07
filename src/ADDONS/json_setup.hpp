#pragma once

#include "cp_color.hpp"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

/**
 * @brief jsf which stands for JSON - Setting - File which has a set of function that control, reutrn, replace a specified Value or sub-directories 
 * @param: getSettings(int  settingsType) - More info provided in the function doxygon
 */
namespace jsf {
  inline constexpr int SETTINGS = 0;
  inline constexpr int DISPLAY_SETTINGS = 1;

  /**
   * @return A json format dictionary which contains specified key's and values for the simulation
   * @param SettingsType The category of settings to retrieve.
   * - 'jsf::SETTINGS': Returns the full JSON Object,
   * - 'jsf::DISPLAY_SETTINGS': Returns only the "display_settings" sub-object.
   * @note Passing integers such as 0 or 1 is also possible as 0 will return the same as 'jsf::SETTINGS' and 1 will also return the same as 'jsf::DISPLAY_SETTINGS'
   * @note Passing a invalid integer will return an empty JSON Object
   */
  inline nlohmann::json getSettings(int settingType) {
    // This 'static' variable lives for the entire duration of the program
    static nlohmann::json cachedSettings; 
    static bool isLoaded = false;

    if (!isLoaded) {
      std::ifstream settingsFile("AppSettings.json");
      if (!settingsFile.is_open()) {
        throw std::runtime_error("Failed to open AppSettings.json! | Error of file at json_setup.hpp");
      }
      settingsFile >> cachedSettings;
      isLoaded = true;
      std::cout << cpc::Green << "Successfully loaded AppSettings.json!" << cpc::Reset << std::endl;
    }

    switch (settingType) {
      case SETTINGS: return cachedSettings;
      case DISPLAY_SETTINGS: return cachedSettings.contains("Display_Settings") ? cachedSettings["Display_Settings"] : nlohmann::json{};
      default: return nlohmann::json{};
    }
  }
}