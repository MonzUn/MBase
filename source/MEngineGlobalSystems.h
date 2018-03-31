#pragma once
#include "Interface/Mengine.h"
#include "Interface/MEngineUtility.h"
#include "MEngineComponentManagerInternal.h"
#include "MEngineConfigInternal.h"
#include "MEngineConsoleInternal.h"
#include "MEngineEntityManagerInternal.h"
#include "MEngineGraphicsInternal.h"
#include "MEngineInternalComponentsInternal.h"
#include "MEngineInputInternal.h"
#include "MEngineSystemManagerInternal.h"
#include "MEngineTextInternal.h"
#include "MEngineUtilityInternal.h"

namespace MEngineGlobalSystems
{
	void Start(const char* applicationName, MEngine::InitFlags initFlags) // TODODB: Check if any system can fail to initialize and return bool accordingly (log inside each system's Initialize() function)
	{
		MEngineUtility::Initialize(applicationName, initFlags);
		MEngineConfig::Initialize();
		MEngineEntityManager::Initialize();
		MEngineComponentManager::Initialize();
		MEngineInternalComponents::Initialize();
		MEngineConsole::Initialize();
		MEngineInput::Initialize();
		MEngineText::Initialize();
		MEngineSystemManager::Initialize();
	}

	void Stop()
	{
		MEngineSystemManager::Shutdown();
		MEngineText::Shutdown();
		MEngineInput::Shutdown();
		MEngineConsole::shutdown();
		MEngineInternalComponents::Shutdown();
		MEngineComponentManager::Shutdown();
		MEngineEntityManager::Shutdown();
		MEngineGraphics::Shutdown(); // TODODB: Place this where it should be after the initialize has been moved in to Start()
		MEngineConfig::Shutdown();
		MEngineUtility::Shutdown();
	}

	void PreEventUpdate()
	{
		MEngineUtility::Update();
		MEngineInput::Update();
	}

	void PostEventUpdate()
	{
		
	}

	void PreSystemsUpdate()
	{

	}

	void PostSystemsUpdate()
	{
		MEngineConsole::Update();
	}
}