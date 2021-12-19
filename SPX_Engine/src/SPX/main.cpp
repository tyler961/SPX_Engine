// Entry point to program. This is a standalone app that can control the engine.


// For project to be completed.
// TODO: Camera
// TODO: Events (First make it so the console says what is clicked)
// TODO: Swapchain resize and recreate
// TODO: Clean up Vulkan API code THIS INCLUDES COMMENTING EVERYTHING TO MAKE IT EASIER LATER
// TODO: Find all TODOs in files and fix it
// TODO: Clean up all code. Remove pragmas, remove ALL warnings.

// After project completed, things to add:
// TODO: im GUI added and set up.
// TODO: Skybox
// TODO: Lighting


#include "Engine.h"
#include "Log.h"

int main()
{
	Log::init();

	Engine engine(1920, 1080, "SPX Engine");
	engine.init();
	engine.run();
	return 0;
}