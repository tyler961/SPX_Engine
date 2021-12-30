#include "Mesh.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "../ThirdParty/tiny_obj_loader.h"
#include "../ThirdParty/vk_mem_alloc.h"
#include "VulkanWrapper/VDevice.h"

Mesh::Mesh(std::string fileLocation, VDevice& device, size_t numSwapChainImages)
	:mDevice(device), mNumSwapChainImages(numSwapChainImages) {
	// Loads model and its vertices and indices.
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fileLocation.c_str())) {
		throw std::runtime_error(warn + err);
	}

	// Gets unique vertices and the indices to reduce the amount of vertices being drawn by the GPU
	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2] };

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(mVertices.size());
				mVertices.push_back(vertex);
			}

			mIndices.push_back(uniqueVertices[vertex]);
		}
	}

	CORE_INFO("Model loaded successfully.");
}

Mesh::~Mesh() {}

void Mesh::createBuffers() {
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
}

void Mesh::createVertexBuffer() {
	// Abstract this later?
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(Vertex) * mVertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	VmaAllocationCreateInfo vmaAllocInfo{};
	vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	if (vmaCreateBuffer(mDevice.mAllocator, &bufferInfo, &vmaAllocInfo, &mVertexBuffer.mBuffer, &mVertexBuffer.mAlloc, nullptr) != VK_SUCCESS)
		CORE_ERROR("Error createing Vertex Buffer in model.");

	//vmaDestroyBuffer(mDevice.mAllocator, mVertexBuffer, mVertexBufferAlloc);

	// Now that I have my memory spot for the vertex data, I copy it from the vertices vector into the GPU readable data.
	void* data;
	vmaMapMemory(mDevice.mAllocator, mVertexBuffer.mAlloc, &data);
	memcpy(data, mVertices.data(), mVertices.size() * sizeof(Vertex));
	vmaUnmapMemory(mDevice.mAllocator, mVertexBuffer.mAlloc);

	CORE_TRACE("Vertex buffer created.");
}

void Mesh::createIndexBuffer() {
	// Same as above? Just for indices? HERES TO HOPING
	// Abstract this later?
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(mIndices[0]) * mIndices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	VmaAllocationCreateInfo vmaAllocInfo{};
	vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	if (vmaCreateBuffer(mDevice.mAllocator, &bufferInfo, &vmaAllocInfo, &mIndexBuffer.mBuffer, &mIndexBuffer.mAlloc, nullptr) != VK_SUCCESS)
		CORE_ERROR("Error createing Index Buffer in model.");

	//vmaDestroyBuffer(mDevice.mAllocator, mIndexBuffer, mIndexBufferAlloc);

	// Now that I have my memory spot for the vertex data, I copy it from the vertices vector into the GPU readable data.
	void* data;
	vmaMapMemory(mDevice.mAllocator, mIndexBuffer.mAlloc, &data);
	memcpy(data, mIndices.data(), (size_t)sizeof(mIndices[0]) * mIndices.size());
	vmaUnmapMemory(mDevice.mAllocator, mIndexBuffer.mAlloc);

	CORE_TRACE("Index buffer created.");
}

void Mesh::createUniformBuffers() {
	mUniformBuffers.resize(mNumSwapChainImages);

	for (size_t i = 0; i < mNumSwapChainImages; i++) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(UniformBufferObject);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		VmaAllocationCreateInfo vmaAllocInfo{};
		vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		if (vmaCreateBuffer(mDevice.mAllocator, &bufferInfo, &vmaAllocInfo, &mUniformBuffers[i].mBuffer, &mUniformBuffers[i].mAlloc, nullptr) != VK_SUCCESS)
			CORE_ERROR("Error creating Uniform Buffer in model.");

		//vmaDestroyBuffer(mDevice.mAllocator, mUniformBuffers[i], mUniformBufferAllocs[i]);
		CORE_TRACE("Uniform buffer created.");
	}
}

void Mesh::updateUniformBuffers(VkExtent2D extent, uint32_t currentImage) {
	// Calculate time in seconds
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo = {};
	// Rotate 90 degress per second
	// existing transform, rotation angle and rotation axis as prams
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(.0f, 1.0f, 1.0f));
	//ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(250.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//ubo.model = glm::mat4(1.0f);
	// Look at the geometry from above at 45 degree angle.
	// eyepos, center pos, up axis are the params
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	// Perspective projection with 45 degree vertial field of view.
	// Next param is aspect ratio, near and far view planes.
	// Important to use current swapchain extent incase the window is resized.
	ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, 1000.0f);
	// GLM has the Y coordinate flipped, so I have to flip it by timesing by -1 or it will be rendered upsidedown
	ubo.proj[1][1] *= -1;

	// All transforms are defined now, so I can copy the data in the uniform buffer obj to the current uniform buffer.
	// This happens the same as vertex buffer, but without the staging buffer becuase it gets called so often, it creates too much overhead
	void* data;
	vmaMapMemory(mDevice.mAllocator, mUniformBuffers[currentImage].mAlloc, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vmaUnmapMemory(mDevice.mAllocator, mUniformBuffers[currentImage].mAlloc);

	// Doing uniform buffers this way is not the most efficent way to pass frequently changing values to the shader.
	// A more efficent way to pass a small buffer of data to shaders are PUSH CONSTANTS.
}
