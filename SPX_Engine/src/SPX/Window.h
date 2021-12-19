#pragma once

#include "../pch.h"

struct GLFWwindow;

class Window
{
public:
	Window(uint32_t width, uint32_t height, std::string title);
	~Window();
	std::string mTitle;
	uint32_t mWidth;
	uint32_t mHeight;

	GLFWwindow* getContext();

private:
	GLFWwindow* mContext;

	
};