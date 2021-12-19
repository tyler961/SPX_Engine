#pragma once

// This class is the control class. It runs and intializes everything.

#include "Window.h"
#include "../pch.h"

class VulkanRenderer;
class Event;

class Engine
{
public:
	// Currently allowing for dynamic creation of window from intializing the engine class.
	// Later it will have to be implicitly called to create the window and give the user
	// better control.
	Engine(uint32_t width, uint32_t height, std::string title);
	~Engine();
	void init();
	void run();
	void createEvents();



	Window mWindow;
	VulkanRenderer* mRenderer;
	std::vector<Event*> mEventsQueue;

private:
};

