#pragma once

// This class is the control class. It runs and intializes everything.


#include "Core.h"
#include "Window.h"
#include "../Platform/Vulkan/Renderer.h"

class Engine
{
public:
	// Currently allowing for dynamic creation of window from intializing the engine class.
	// Later it will have to be implicitly called to create the window and give the user
	// better control.
	Engine(SPX_INT width, SPX_INT height, std::string title);
	~Engine();
	void init();
	void run();


	Window mWindow;
	Renderer mRenderer;


private:
	void drawFrame();

};

