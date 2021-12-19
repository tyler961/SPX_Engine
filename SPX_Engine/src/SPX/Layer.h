#pragma once

#include "../Events/Event.h"
#include "../pch.h"

// Layer class is a pure virtual class.
// The engine uses the layer functions and the layer classes abstracted from here can control those functions.
// This can be used for the application project and things like imGUI

// TODO: Put namspaces around things so the client application knows what theyre using.

class Layer
{
public:
	Layer(const std::string& name = "Layer");
	virtual ~Layer();

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnUpdate() {}
	virtual void OnImGuiRender() {}
	virtual void OnEvent(Event& event) {}

	inline const std::string& GetName() const { return mDebugName; }

private:
	std::string mDebugName;
};