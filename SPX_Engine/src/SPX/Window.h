#pragma once

#include "../pch.h"

struct GLFWwindow;
class Event;

class Window {
public:
	Window(uint32_t width, uint32_t height, std::string title);
	~Window();
	std::string mTitle;
	uint32_t mWidth;
	uint32_t mHeight;

	GLFWwindow* getContext();
	void pollEvents();
	// Update later
	void createKeyboardEvent(int key, int action);

	std::vector<Event*> mEventsQueue;

private:
	GLFWwindow* mContext;
};