#include "../pch.h"
#include "Engine.h"

Engine::Engine(SPX_INT width, SPX_INT height, std::string title)
	:mWindow(Window(width, height, title)), mRenderer(Renderer(&mWindow))
{
}

Engine::~Engine()
{
}

void Engine::init()
{
}

void Engine::run()
{
	while (!glfwWindowShouldClose(mWindow.getContext()))
	{
		glfwPollEvents();
	}
}