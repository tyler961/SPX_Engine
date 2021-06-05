// Entry point to program. This is a standalone app that can control the engine.
#include "../pch.h"
#include "Engine.h"

int main()
{
	Log::init();

	Engine engine(1920, 1080, "SPX Engine");
	engine.run();
	return 0;
}