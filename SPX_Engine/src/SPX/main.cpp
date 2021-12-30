// Entry point to program. This is a standalone app that can control the engine.

// After project completed, things to add:
// TODO: Cleanup any and all warnings
// TODO: Add callbacks for every type of event, not just keyboard events
// TODO: Swapchain resize and recreate
// TODO: Add more dynamic choices within the graphicspipeline that the engine can change.
// TODO: Allow dynamic addition of render objects
// TODO: Place engine code into namespaces
// TODO: Divide application code from actual engine so app can use engine but not change it
// TODO: im GUI added and set up.
// TODO: Make events more robust. Change event class into a virtual class where other event classes derive from it
// TODO: Create base classes that objects (camera, rendered objects, meshes, etc) can derive from
// TODO: Move render object to actors.

#include "Engine.h"
#include "Log.h"

int main() {
	Log::init();

	Engine engine(1920, 1080, "SPX Engine");
	engine.init();
	engine.run();
	return 0;
}