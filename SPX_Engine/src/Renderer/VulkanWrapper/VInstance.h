#pragma once

#include "../../pch.h"

class VulkanValidationLayers;

class VInstance {
public:
	VInstance(std::string& appName, std::string& engineName, bool enableValidationLayers);
	~VInstance();

	// Getter
	VkInstance get() const { return mInstance; }

	// TODO : Set to optional instead of pointer.
	//std::optional<VulkanValidationLayers> mValLayers{ std::nullopt };
	VulkanValidationLayers* mValLayers{ nullptr };

private:
	VkInstance mInstance;
};