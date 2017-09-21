#pragma once

namespace MEngine
{
	bool				Initialize(const char* appName);
	void				Shutdown();
	
	bool				IsInitialized();
	bool				ShouldQuit();
	
	void				Update();
	void				Render();
};