#include "Shader.h"

Shader::Shader(ShaderType type, std::string file, VkDevice device)
	:mType(type), mFileName(file)
{
	readFile();
	createShaderModule(device);
}

void Shader::readFile()
{
	// ate startes reading at the end. This is important so I can use read position to determine the file size and allocate a buffer
	std::ifstream file(mFileName, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		throw std::runtime_error("Failed to open shader file" + mFileName);

	// Allocate buffer based on filesize
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	// Now I can seek back to the beginning of the file and read all the bytes at once
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	// Close the file and return the bytes
	file.close();

	mShaderCode = buffer;
}

void Shader::createShaderModule(VkDevice device)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = mShaderCode.size();
	// Have to cast because it is a char when I need uint32_t
	createInfo.pCode = reinterpret_cast<const SPX_INT*>(mShaderCode.data());

	if (vkCreateShaderModule(device, &createInfo, nullptr, &mShaderModule) != VK_SUCCESS)
		CORE_ERROR("Failed to create shader module.");
	else
		CORE_INFO("Shader Module created successfully.");
}
