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


#define EVENT_CLASS_TYPE(type)  static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType getType() const override { return GetStaticType(); }\
								virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int getCategory() const override { return category; }\


class Event
{
	friend class EventDispatcher;
public:
	bool mIsHandled = false;

	virtual int getCategory() const = 0;
	virtual EventType getType() const = 0;
	virtual const char* getName() const = 0;
	virtual std::string ToString() const { return getName(); }

	inline bool isInCategory(EventCategory category)
	{
		return getCategory() & category;
	}
};

class EventDispatcher
{
	template<typename T>
	using EventFn = std::function<bool(T&)>;
public:
	EventDispatcher(Event& event)
		:mEvent(event)
	{}

	template<typename T>
	bool Dispatch(EventFn<T> func)
	{
		if (mEvent.getType() == T::GetStaticType())
		{
			mEvent.mIsHandled = func(*(T*) &mEvent);
			return true;
		}
		return false;
	}
private:
	Event& mEvent;
};

inline std::ostream& operator<<(std::ostream& os, const Event& e)
{
	return os << e.ToString();
}