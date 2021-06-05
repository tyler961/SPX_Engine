#pragma once

#include "../../pch.h"

// Struct to send to the vertex shader instead of hardcoding it into the shader
struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		// Vertex binding describes at which rate to load data from memory throughout the vertices.
		// Specifies the number of bytes between data entires and whether ot move to the net data entry after each vertex or instance
		VkVertexInputBindingDescription bindingDescription{};
		// For now all the pre-vertex data is packed together in one array, so only have to have one binding.
		// Binding parameter specifies the index of the binding in the array of bindings.
		// Stripe parameter specifies the number of bytes from one entry to the next
		// InputRate can have two different values:
		// VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
		// VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance.
		// Not using instance rendering, so per-vertex data
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	// This describes how to extract a vertex attribute from a chunk of vertex data from a binding description.
	// I have two attributes, position and color, so I need two attribute description structs.
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		// Binding tells Vulkan from which binding the per-vertex data comes.
		// Location references the location directive of the input in the vertex shader
		// Format describes the type of data for the attribute. These are enumerations of color formats. Following available:
		// float: VK_FORMAT_R32_FLOAT
		// vec2: VK_FORMAT_R32G32_SFLOAT
		// vec3: VK_FORMAT_R32G32B32_SFLOAT
		// vec4: VK_FORMAT_R32G32B32A32_SFLOAT
		// the last part SFLOAT needs to match the bit width in the shader. Pg 156, ex:  ivec2: VK_FORMAT_R32G32_SINT
		// offset specifies the number of bytes since the start of the pre-vertex data to read from. pg 156
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		// Same as above but for the next attribute, color instead of position
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

// Uniform Buffer Object. Explained more on page 180
struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

namespace std
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}