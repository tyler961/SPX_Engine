#pragma once

#include <GLFW/glfw3.h>
#include "Core.h" 

class Window
{
public:
	Window(SPX_INT width, SPX_INT height, std::string title);
	~Window();
	std::string mTitle;
	SPX_INT mWidth;
	SPX_INT mHeight;

	GLFWwindow* getContext();

private:
	GLFWwindow* mContext;

	
};