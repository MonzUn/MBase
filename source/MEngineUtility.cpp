#include "Interface/MEngineUtility.h"
#include "MEngineUtilityInternal.h"
#include "MEngineGraphicsInternal.h"
#include <MUtilitySystem.h>
#include <SDL.h>
#include <atomic>

using namespace MEngine;
using namespace MEngineUtility;

namespace MEngineUtility
{
	const std::string*		m_ApplicationName	= nullptr;
	const std::string*		m_ExecutablePath	= nullptr;
	InitFlags				m_InitFlags			= InitFlags::None;

	std::atomic<bool>	m_HasFocus = false;
	std::atomic<bool>	m_IsHovered = false;
}

// ---------- INTERFACE ----------

const std::string& MEngine::GetApplicationName()
{
	return *m_ApplicationName;
}

const std::string& MEngine::GetExecutablePath()
{
	return *m_ExecutablePath;
}

const MEngine::InitFlags MEngine::GetInitFlags()
{
	return m_InitFlags;
}

bool MEngine::WindowHasFocus()
{
	return m_HasFocus;
}

bool MEngine::WindowIsHovered()
{
	return m_IsHovered;
}

// ---------- INTERNAL ----------

void MEngineUtility::Initialize(const char* applicationName, InitFlags initFlags)
{
	m_ApplicationName	= new std::string(applicationName);
	m_ExecutablePath	= new std::string(MUtility::GetExecutableDirectoryPath());
	m_InitFlags			= initFlags;
	m_HasFocus			= SDL_GetWindowFlags(MEngineGraphics::GetWindow()) & SDL_WINDOW_INPUT_FOCUS;
	m_IsHovered			= SDL_GetWindowFlags(MEngineGraphics::GetWindow()) & SDL_WINDOW_MOUSE_FOCUS;
}

void MEngineUtility::Shutdown()
{
	delete m_ApplicationName;
	delete m_ExecutablePath;
}

void MEngineUtility::Update()
{
	m_HasFocus	= SDL_GetWindowFlags(MEngineGraphics::GetWindow()) & SDL_WINDOW_INPUT_FOCUS;
	m_IsHovered	= SDL_GetWindowFlags(MEngineGraphics::GetWindow()) & SDL_WINDOW_MOUSE_FOCUS;
}