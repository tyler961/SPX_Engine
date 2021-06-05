#pragma once

#include "../../pch.h"

// TODO:: add shader compiler here so I just send the shader file instead of the spv file.

enum class ShaderType { NONE, VERT_SHADER, FRAG_SHADER };

class Shader
{
public:
	Shader(ShaderType type, std::string file, VkDevice device);

	void readFile();
	void createShaderModule(VkDevice device);


	std::string mFileName;
	std::vector<char> mShaderCode;
	VkShaderModule mShaderModule;
	ShaderType mType;
};