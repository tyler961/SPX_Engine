#pragma once

#include "../../pch.h"

enum class ShaderType { NONE, VERT_SHADER, FRAG_SHADER };

// TODO: Add shader compiler so I can send the shader file instead of the spv file.

class VDevice;

class VShader {
public:
	VShader(ShaderType type, std::string file, VDevice& device);
	~VShader();

	void readFile(std::string fileName);
	void createShaderModule();

	VkShaderModule mShaderModule{ VK_NULL_HANDLE };
	ShaderType mType;

private:
	std::vector<char> mShaderCode;
	VDevice& mDevice;
};