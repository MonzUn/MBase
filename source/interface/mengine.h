#pragma once

namespace MEngine
{
	bool				Initialize();
	void				Shutdown();
	
	bool				IsInitialized();
	bool				ShouldQuit();
	
	void				Update();
	void				Render();
};