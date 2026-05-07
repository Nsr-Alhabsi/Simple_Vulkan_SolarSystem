#pragma once
#include <string>

/** 
 * @brief returns a specific string code which changes the following text to the specified color.
 * @return A string code i.e "\033[31m"
 * 
 * @details The codes are used differently depending on the color.
 * - use Red to indicate a Error in the code
 * - use Green to indicate a successful action in the code
 * - use Yellow to indicate a warning in the code
 * - use Cyan to indicate a infomation in the code
 * - use White to indicate a normal text in the code
 * - use Reset to reset the color back to normal after a colored text this always comes at the end of a colored text
*/
namespace cpc { 
  extern const std::string Red; 
  extern const std::string Green; 
  extern const std::string Yellow;
  extern const std::string Cyan;
  extern const std::string White;
  extern const std::string Reset;
}
