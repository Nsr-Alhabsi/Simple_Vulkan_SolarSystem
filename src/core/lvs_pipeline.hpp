#pragma once

#include "lvs_device.hpp"

#include <string>
#include <vector>

namespace lvs {


/**
 * @brief A class representing a Vulkan graphics pipeline, which is responsible for managing the various stages of the rendering process, including shader modules and pipeline configuration.
 * 
 * @details The LvsPipeline class encapsulates the creation and management of the Vulkan graphics pipeline, including the loading of shader modules and the configuration of the pipeline stages. It provides functionality for reading shader code from files, creating shader modules, and setting up the graphics pipeline based on provided configuration information. The class also ensures that resources are properly managed and cleaned up when the pipeline is destroyed.
 * 
 * @param PipelineConfigInfo A structure that holds configuration information for the graphics pipeline, such as viewport settings, rasterization state, and other pipeline parameters. This allows for flexible configuration of the pipeline based on the specific needs of the application.
 * @param readFile a static function that reads the contents of a file and returns it as a vector of characters, which is commonly used for loading shader code in Vulkan applications. The function takes a file path as input, attempts to open the file, and reads its contents into a buffer. If the file cannot be opened or read successfully, it throws a runtime error with an appropriate message. If the file is read successfully, it returns the conents as a vector of characters.
 * @param createGraphicsPipeline a function that creates the graphics pipeline based on the provided vertex and fragment shader file paths as well as the pipeline configuration information. This function is responsible for setting up the various stages of the graphics pipeline, including shader stages, fixed-function stages, and pipeline layout.
 * @param createShaderModule a function that creates a Vulakn shader module from the provided shader code. This function takes the shader code as a vector of characters and creates a Vulkan shader module, which can then be used in the graphics pipeline. The function handles the creation of the shader module and ensures that it is properly set up for use in the pipeline.
 * @param lvsDevice a reference to the LvsDevice class, which provides access to the Vulkan device and other related resources needed for creating the graphics pipeline. This allows the LvsPipeline class to interact with the Vulkan device and utlize its functionality for creating and managing the graphics pipeline.
 * @param graphicsPipeline a Vulkan pipeline object that represents the graphics pipeline created by this class. This object is used to manage the various stages of the rendering process and is essential for rendering operations in Vulkan applications.
 * @param vertShaderModule a Vulkan shader module that represents the vertex shader used in the graphics pipeline this module is responsible for creating and managing the vertex shader module, which is a crucial component of the graphics pipeline. The vertex shader processes vertex data and is an essential part of the rendering process in Vulkan applications.
 * @param fragShaderModule a Vulkan shader module that represents the fragment shader used in the graphics pipeline. This module is responsible for creating and managing the fragment shader module which is a crucial component of the graphics pipeline. The fragment shader processes fragment data and is an essential part of the rendering process in vulkan applications.
 * 
 * @bug If the shader files specified by vertFilePath and fragFilePath cannot be found or read successfully, the createGraphicsPipeline function will throw a runtime error with an appropriate message. This ensures that any issues with loading the shader code are properly reported and can be handled by the calling code.
 * 
 */
class LvsPipeline {
public:
  struct PipelineConfigInfo {
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStatEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
  };

  /**
   * @brief Constructs a new LvsPipeline object, which is responsible for creating and managing a Vulkan graphics pipeline based on the provided vertex and fragment shader file paths as well as the pipeline configuration information. The constructor initializes the graphics pipeline by calling the createGraphicsPipeline function, which sets up the various stages of the pipeline and loads the shader modules. The constructor also takes a reference to the LvsDevice class, which provides access to the Vulkan device and other related resources needed for creating the graphics pipeline.
   * 
   * @details The constructor initializes the graphics pipeline by calling the createGraphicsPipeline function, which sets up the various stages of the pipeline and loads the shader modules. The constructor also takes a reference to the LvsDevice class, which provides access to the Vulkan device and other related resources needed for creating the graphics pipeline. If any issues arise during the creation of the graphics pipeline (e.g., shader files cannot be loaded, pipeline configuration is invalid), the constructor will throw a runtime error with an appropriate message, ensuring that any problems are properly reported and can be handled by the calling code.
   * @details How the code works line by line:
   * - Step 1: The constructor takes three parameters: a reference to an LvsDevice object, the file paths for the vertex and fragment shaders, and a PipelineConfigInfo structure that contains configuration information for the graphics pipeline.
   * - Step 2: The constructor initializes the graphics pipeline by calling the createGraphicsPipeline function, passing in the vertex and framgent shader file paths as well as the pipeline configuration information. This function is responsible for setting up the various stages of the graphics pipeline, including shader stages, fixed-function stages and the pipeline layout.
   * - Step 3: If any issues arise during the creation of the graphics pipeline (e.g., shader files cannot be loaded, pipeline configuration is invalid), the createGraphicsPipeline function will throw a runtime error with an appropriate messagae. This ensures that any problems encountered during the pipeline creation process are properly reported and can be handled by the calling code, such as by logging the error or providing feedback to the user.
   * - Step 4: If the graphics pipeline is created successfully, the constructor completes without throwing any exceptions, and the LvsPipeline object is ready to be used for rendering operations in the Vulkan application.
   * 
   * @param device - A reference to an LvsDevice object, which provides access to the vulkan device and other related resources needed for creating the graphics pipeline. This allows the LvsPipeline class to interact with the Vulkan device and utilize its functionality for creating and managing the graphics pipeline.
   * @param vertFilePath - The file path to the vertex shader code that will be used in the graphics pipeline. This should be a valid path to a shader file on the filesystem, and the file should contain valid shader code that can be compiled into a Vulkan shader module.
   * @param fragFilePath  - The file path to the fragment shader code that will be used in the graphics pipeline. Similar to vertFilePath, this should be a valid path to a shader file on the filesystem, and the file should contain valid shader code that can be compiled into a Vulkan shader module.
   * @param configInfo - A PipelineConfigInfo structure that contains configuration information for the graphics pipeline, such as viewport settings, rasterization state, and other pipeline parameters. This allows for flexible configuration of the pipeline based on the specific needs of the application. 
   */
  LvsPipeline(
    LvsDevice& device,
    const std::string& vertFilePath,
    const std::string& fragFilePath,
    const PipelineConfigInfo& configInfo);
  
  /**
   * @brief Destroy the Lvs Pipeline object
   * 
   */
    ~LvsPipeline();

  LvsPipeline(const LvsPipeline&) = delete;
  LvsPipeline& operator=(const LvsPipeline&) = delete;

  void bind(VkCommandBuffer commandBuffer);

  /**
   * @brief Creates a default pipeline configuration info object with the specified width and height. This function provides a convenient way to generate a default configuration for the graphics pipeline based on the dimensions of the rendering surface. The returned PipelineConfigInfo object can be used as a starting point for configuring the graphics pipeline, allowing developers to easily set up the pipeline with reasonable default settings for viewport, rasterization state, and other parameters based on the provided width and height.
   * 
   * @param width - width of the window this can be changed in the AppSettings.json file under the Screen object. This allows for flexible configuration of the pipeline based on the specific needs of the application and the dimensions of the rendering surface.
   * @param height  - height of the window this can be changed in the AppSettings.json file under the Screen object. This allows for flexible configuration of the pipeline based on the specific needs of the application and the dimensions of the rendering surface.
   * @return PipelineConfigInfo  - A PipelineConfigInfo object that contains default configuration settings for the graphics pipeline based on the specified width and height. This object can be used as a starting point for configuring the graphics pipeline in a Vulkan application.
   */
  static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);
private:
  /**
   * @brief Reads the contents of a file and returns it as a vector of characters, which is commonly used for loading shader code in Vulkan applications.
   * 
   * @details The function takes a file path as input, attempts to open the file, and reads its contents into a buffer. If the file cannot be opened or read successfully, it throws a runtime error with an appropriate message. If the file is read successfully, it returns the contents as a vector of characters.
   * 
   * @param filePath
   * - The path to the file that needs to be read. This should be a valid path to a file on the filesystem
   * 
   * @bug If the file cannot be opened (e.g., due to an incorrect path, missing file, or insufficient permissions), the function will throw a runtime error with a message indicating the failure to open the file. This ensures that the calling code can handle the error appropriately, such as by logging the error or providing feedback to the user.
   * @bug If the file is successfully opened but cannot be read (e.g., due to an I/O error), the function will throw a runtime error with a message indicating the failure to read the file. This ensures that any issues encountered during the reading process are properly reported and can be handled by the calling code.
   *  
   * @return std::vector<char> 
   */
  static std::vector<char> readFile(const std::string& filePath);

  /**
   * @brief Creates the graphics pipeline based on the provided vertex and fragment shader file paths as well as the pipeline configuration information. This function is responsible for setting up various stages of the graphics pipeline, including shader stages, fixed-function stages, and pipeline layout. It loads the shader code from the specified files, creates shader modules, and configures the graphics pipeline according to the provided configuration information. If any issues arise during the creation of the graphics pipeline (e.g., shader files cannot be loaded), the function will throw a runtime error with an appropriate message, ensuring that any problems are properly reported and can be handled by the calling code.
   * 
   * @param vertFilePath  -- The file path to the vertex shader code that will be used in the graphics pipeline. This should be a valid path to a shader file on the filesystem, and the file should contain vaild shader code that can be compiled into a Vulkan shader module.
   * @param fragFilePath  -- The file path to the fragment shader code that will be used in the graphics pipeline. Similar to vertFilePath, this should be a valid path to a shader file on the filesystem, and the file should contain valid shader code that can be compiled into a Vulkan shader module.
   * @param configInfo -- A PipelineConfigInfo structure that contains configuration information for the graphics pipeline, such as viewport settings, rasterization state, and other pipeline parameters. This allows for flexible configuration of the pipeline based on the specific needs of the application. 
   */
  void createGraphicsPipeline(
    const std::string& vertFilePath,
    const std::string& fragFilePath,
    const PipelineConfigInfo& configInfo);
  
  void createShaderModule(const std::vector<char>& code, VkShaderModule* VkShaderModule);

  LvsDevice& lvsDevice;
  VkPipeline graphicsPipeline;
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
};

}