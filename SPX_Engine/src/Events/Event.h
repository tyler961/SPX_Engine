#pragma once

#include "../pch.h"

// For now, events are held within a queue in the engine. When an event happens, it is created and added to the vector.
// Anything that will want to have events active, will get that info passed during handle events. It will look through 
// and see if any events are ones that it wants to handle.


enum class EventType
{
	None = 0,
	WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
	AppTick, AppUpdate, AppRender,
	KeyPressed, KeyReleased, KeyTyped,
	MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

enum EventCategory
{
	None = 0,
	EventCategoryApplication	= (1 << 0),  // 0000 0001
	EventCategoryInput			= (1 << 1),  // 0000 0010
	EventCategoryKeyboard		= (1 << 2),  // 0000 0100
	EventCategoryMouse			= (1 << 3),  // 0000 1000
	EventCategoryMouseButton	= (1 << 4)   // 0001 0000
};

// Using flags example:
// Combined types to make multiples:	int combinedFlags = EventCategoryApplication | EventCategoryInput;
// Check if it contains a flag:			bool hasFlag = combinedFlags & EventCategoryInput; (WILL BE TRUE)
// If it doesnt have the flag:			bool hasFlag = combinedFlags & EventCategoryKeyboard; (WILL BE FALSE)


// Basic event structure for now. Can expand later. For now it is just a wrapper for GLFW information.
class Event {
public:
	Event(int keycode, EventType type, int category)
		: mKeycode(keycode), mType(type), mCategory(category), mIsHandled(false) {};

	int getKeycode() { return mKeycode; }
	EventType getType() { return mType; }
	int getCategory() { return mCategory; }
	bool getIsHandled() { return mIsHandled; }

	// Setter
	void setIsHandled(bool handled) { mIsHandled = handled; }

private:
	int mKeycode;
	EventType mType;
	int mCategory;
	bool mIsHandled;
};