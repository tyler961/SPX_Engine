#pragma once

// This class is the control class. It runs and intializes everything.
#include "../pch.h"
#include "Window.h"

class VulkanRenderer;
class Camera;

class Engine {
public:
	// Currently allowing for dynamic creation of window from intializing the engine class.
	// Later it will have to be implicitly called to create the window and give the user
	// better control.
	Engine(uint32_t width, uint32_t height, std::string title);
	~Engine();
	void init();
	void run();
	void handleEvents();
	void update();

	Window mWindow;
	VulkanRenderer* mRenderer;
	Camera* mCamera;

private:
};

