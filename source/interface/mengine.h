#pragma once
#include <stdint.h>

namespace MEngine
{
	bool				Initialize(const char* appName = "MEngineApp", int32_t windowWidth = 1024, int32_t windowHeight = 768);
	void				Shutdown();
	
	bool				IsInitialized();
	bool				ShouldQuit();
	
	void				Update();
	void				Render();
};