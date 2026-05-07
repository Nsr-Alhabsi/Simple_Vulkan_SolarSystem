#include "lvs_pipeline.hpp"
#include "../ADDONS/cp_color.hpp"
#include "../ADDONS/json_setup.hpp"
#include "../models/lvs_model.hpp"

#include <fstream>
#include <iostream>
#include <exception>
#include <cassert>

#include <nlohmann/json.hpp>

namespace lvs {

LvsPipeline::LvsPipeline(
    LvsDevice& device,
    const std::string& vertFilePath,
    const std::string& fragFilePath,
    const PipelineConfigInfo& configInfo) : lvsDevice{device} {
  createGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
}

LvsPipeline::~LvsPipeline() {
  vkDestroyShaderModule(lvsDevice.device(), vertShaderModule, nullptr);
  vkDestroyShaderModule(lvsDevice.device(), fragShaderModule, nullptr);
  vkDestroyPipeline(lvsDevice.device(), graphicsPipeline, nullptr);
}

std::vector<char> LvsPipeline::readFile(const std::string& filePath) {
  std::ifstream file{filePath, std::ios::ate | std::ios::binary};

  if(!file.is_open()) {
    throw std::runtime_error("Failed to open file!: " + filePath);
  }
  std::cout << cpc::Green << "Successfully opened file!: " + filePath + cpc::Reset + "\n";

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  std::cout << cpc::White << "File read successfully returning buffer." << cpc::Reset << std::endl;
  return buffer;
}

void LvsPipeline::createGraphicsPipeline(
    const std::string& vertFilePath,
    const std::string& fragFilePath,
    const PipelineConfigInfo& configInfo) {
  auto vertShaderCode = readFile(vertFilePath);
  auto fragShaderCode = readFile(fragFilePath);

  std::cout << cpc::Cyan << "Vertex Shader Code Size: " << vertShaderCode.size() << " bytes" << cpc::Reset << std::endl;
  std::cout << cpc::Cyan << "Fragment Shader Code Size: " << fragShaderCode.size() << " bytes" << cpc::Reset << std::endl;

  assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no pipelineLayout provided in configInfo");
  assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no renderPass provided in configInfo");

  createShaderModule(vertShaderCode, &vertShaderModule);
  createShaderModule(fragShaderCode, &fragShaderModule);

  VkPipelineShaderStageCreateInfo shaderStages[2];
                                        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].stage               = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module              = vertShaderModule;
  shaderStages[0].pName               = "main";
  shaderStages[0].flags               = 0;
  shaderStages[0].pNext               = nullptr;
  shaderStages[0].pSpecializationInfo = nullptr;
  
                                        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module              = fragShaderModule;
  shaderStages[1].pName               = "main";
  shaderStages[1].flags               = 0;
  shaderStages[1].pNext               = nullptr;
  shaderStages[1].pSpecializationInfo = nullptr;

  auto bindingDescriptions = LvsModel::Vertex::getBindingDescriptions();
  auto attributeDescriptions = LvsModel::Vertex::getAttributeDescriptions();
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexAttributeDescriptionCount       = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.vertexBindingDescriptionCount         = static_cast<uint32_t>(bindingDescriptions.size());
  vertexInputInfo.pVertexAttributeDescriptions          = attributeDescriptions.data();
  vertexInputInfo.pVertexBindingDescriptions            = bindingDescriptions.data();

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount          = 2;
  pipelineInfo.pStages             = shaderStages;
  pipelineInfo.pVertexInputState   = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
  pipelineInfo.pViewportState      = &configInfo.viewportInfo;
  pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
  pipelineInfo.pMultisampleState   = &configInfo.multisampleInfo;
  pipelineInfo.pColorBlendState    = &configInfo.colorBlendInfo;
  pipelineInfo.pDepthStencilState  = &configInfo.depthStencilInfo;
  pipelineInfo.pDynamicState       = &configInfo.dynamicStateInfo;

  pipelineInfo.layout              = configInfo.pipelineLayout;
  pipelineInfo.renderPass          = configInfo.renderPass;
  pipelineInfo.subpass             = configInfo.subpass;

  pipelineInfo.basePipelineIndex   = -1;
  pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

  VkResult result = vkCreateGraphicsPipelines(lvsDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
  if (result != VK_SUCCESS) {
    throw std::runtime_error(cpc::Red + "Failed to create graphics pipeline, Error: " + std::to_string(result) + cpc::Reset);
  }
  std::cout << cpc::Green << "Successfully created graphics pipeline!" << cpc::Reset << std::endl;
}

void LvsPipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());


  VkResult result = vkCreateShaderModule(lvsDevice.device(), &createInfo, nullptr, shaderModule);
  if(result != VK_SUCCESS) {
    throw std::runtime_error(cpc::Red + "Failed to create Shader module, Error: " + std::to_string(result) + cpc::Reset);
  }
  std::cout << cpc::Green << "Successfully created shader module" << cpc::Reset << std::endl;
}

void LvsPipeline::bind(VkCommandBuffer commandBuffer) {
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}

void LvsPipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {
  configInfo.inputAssemblyInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  configInfo.inputAssemblyInfo.topology                = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  configInfo.inputAssemblyInfo.primitiveRestartEnable  = VK_FALSE;

  configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  configInfo.viewportInfo.viewportCount = 1;
  configInfo.viewportInfo.scissorCount = 1;

  configInfo.rasterizationInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  configInfo.rasterizationInfo.depthClampEnable        = VK_FALSE;
  configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  configInfo.rasterizationInfo.polygonMode             = VK_POLYGON_MODE_FILL;
  configInfo.rasterizationInfo.lineWidth               = 1.0f;
  configInfo.rasterizationInfo.cullMode                = VK_CULL_MODE_NONE;
  configInfo.rasterizationInfo.frontFace               = VK_FRONT_FACE_CLOCKWISE;
  configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
  configInfo.rasterizationInfo.depthBiasClamp          = 0.0f; // Optional
  configInfo.rasterizationInfo.depthBiasSlopeFactor    = 0.0f; // Optional

  configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  configInfo.colorBlendAttachment.colorWriteMask       = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  configInfo.colorBlendAttachment.blendEnable          = VK_FALSE;
  configInfo.colorBlendAttachment.srcColorBlendFactor  = VK_BLEND_FACTOR_ONE;
  configInfo.colorBlendAttachment.dstColorBlendFactor  = VK_BLEND_FACTOR_ZERO;
  configInfo.colorBlendAttachment.colorBlendOp         = VK_BLEND_OP_ADD;
  configInfo.colorBlendAttachment.srcAlphaBlendFactor  = VK_BLEND_FACTOR_ONE;
  configInfo.colorBlendAttachment.dstAlphaBlendFactor  = VK_BLEND_FACTOR_ZERO;
  configInfo.colorBlendAttachment.alphaBlendOp         = VK_BLEND_OP_ADD;

  configInfo.colorBlendInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  configInfo.colorBlendInfo.logicOpEnable              = VK_FALSE;
  configInfo.colorBlendInfo.logicOp                    = VK_LOGIC_OP_COPY; // Optional
  configInfo.colorBlendInfo.attachmentCount            = 1;
  configInfo.colorBlendInfo.pAttachments               = &configInfo.colorBlendAttachment;
  configInfo.colorBlendInfo.blendConstants[0]          = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[1]          = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[2]          = 0.0f; // Optional
  configInfo.colorBlendInfo.blendConstants[3]          = 0.0f; // Optional
  
  configInfo.depthStencilInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  configInfo.depthStencilInfo.depthTestEnable          = VK_TRUE;
  configInfo.depthStencilInfo.depthWriteEnable         = VK_TRUE;
  configInfo.depthStencilInfo.depthCompareOp           = VK_COMPARE_OP_LESS;
  configInfo.depthStencilInfo.depthBoundsTestEnable    = VK_FALSE;
  configInfo.depthStencilInfo.minDepthBounds           = 0.0f; // Optional
  configInfo.depthStencilInfo.maxDepthBounds           = 1.0f; // Optional
  configInfo.depthStencilInfo.stencilTestEnable        = VK_FALSE;
  configInfo.depthStencilInfo.front                    = {}; // Optional
  configInfo.depthStencilInfo.back                     = {}; // Optional
  
  configInfo.dynamicStatEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStatEnables.data();
  configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStatEnables.size());
  configInfo.dynamicStateInfo.flags = 0;
  std::cout << cpc::Green << "Successfully created the Pipeline config info" << cpc::Reset << std::endl; 
}

} // namespace lvs