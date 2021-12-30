#include "../pch.h"
#include "Engine.h"
#include "../Events/Event.h"
#include "../Renderer/VulkanRenderer.h"
#include "../Renderer/RenderObject.h"
#include "Camera.h"
#define VMA_IMPLEMENTATION
#include "../ThirdParty/vk_mem_alloc.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>


Engine::Engine(uint32_t width, uint32_t height, std::string title)
	:mWindow(Window(width, height, title)), mRenderer(new VulkanRenderer(&mWindow)), mCamera(new Camera()) {}

Engine::~Engine() {}

void Engine::init() {
	RenderObject tmp("Media/Obj/chalet.obj", "Media/Textures/chalet.jpg");
	mRenderer->addRenderObject(tmp);
	tmp = RenderObject("Media/Obj/viking.obj", "Media/Textures/viking.png");
	mRenderer->addRenderObject(tmp);
	mRenderer->init("Test App", "SPX_ENGINE", true);
}

void Engine::run() {
	// Find cleaner wait to limit the fps. This works for now to prevent excessive loops and placing strain on the GPU.
	std::chrono::time_point<std::chrono::system_clock> start, end = std::chrono::system_clock::now();
	std::chrono::duration<double, std::milli> passedTime;
	
	while (!glfwWindowShouldClose(mWindow.getContext())) {
		// Frame limiter
		start = std::chrono::system_clock::now();
		passedTime = end - start;
		// 7ms =~ 144 fps
		if (passedTime.count() < 7) {
			std::chrono::duration<double, std::milli> deltaMs(7 - passedTime.count());
			auto deltaMsDuration = std::chrono::duration_cast<std::chrono::milliseconds>(deltaMs);
			std::this_thread::sleep_for(std::chrono::milliseconds(deltaMsDuration.count()));
		}
		end = std::chrono::system_clock::now();	
		// End frame limiter code


		// Actual engine code.
		mWindow.pollEvents();
		handleEvents();
		update();
		mRenderer->draw(mCamera->getViewMatrix());
	}
}

// Update this over time with more events. (These events are the events the engine wants to handle.
// The rest it sends to other objects to handle the events as they want.
void Engine::handleEvents() {
	for (size_t i = 0; i < mWindow.mEventsQueue.size(); i++) {
		// Delete handled events. This may change later depending on if multiple objects want to handle the event before it is deleted.
		if (mWindow.mEventsQueue.at(i)->getIsHandled()) {
			mWindow.mEventsQueue.erase(mWindow.mEventsQueue.begin() + i);
		}
		else {
			// Events I want the engine to specifically handle.
		}
	}
	mCamera->handleEvents(mWindow.mEventsQueue);
}

// This is used to update any number of things.
// For now it will update the camera and the render objects.
void Engine::update() {
	mCamera->updateCamera();
}