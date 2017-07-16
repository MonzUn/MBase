#pragma once

struct SDL_Window;
struct SDL_Renderer;

class MEngine
{
public:
	bool Initialize();
	void Shutdown();

	bool ShouldQuit() const;

	void Update();
	void Render();

private:
	bool				m_Initialized	= false;
	SDL_Window*			m_Window		= nullptr;
	SDL_Renderer*		m_Renderer		= nullptr;
	bool				m_QuitRequested = false;
};