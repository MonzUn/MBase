#pragma once
#include "MengineTypes.h"
#include <stdint.h>

namespace MEngine
{
	constexpr int32_t DEFAULT_WINDOW_POS_X	= 0;
	constexpr int32_t DEFAULT_WINDOW_POS_Y	= 0;
	constexpr int32_t DEFAULT_WINDOW_WIDTH	= 1024;
	constexpr int32_t DEFAULT_WINDOW_HEIGHT = 768;

	bool Initialize(const char* applicationName = "MEngineApp", InitFlags initFlags = InitFlags::None);
	bool CreateWindow(const char* windowTitle = "", int32_t windowPosX = DEFAULT_WINDOW_POS_X, int32_t windowPosY = DEFAULT_WINDOW_POS_Y, int32_t windowWidth = DEFAULT_WINDOW_WIDTH, int32_t windowHeight = DEFAULT_WINDOW_HEIGHT);
	void Shutdown();
		 
	bool IsInitialized();
	bool ShouldQuit();
		 
	void Update();
	void Render();
};