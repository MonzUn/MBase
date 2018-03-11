#pragma once
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
	void Start() // TODODB: Check if any systm can fail to initialize and return bool accordingly (log inside each system's Initialize() function)
	{
		MEngineUtility::Initialize();
		MEngineEntityManager::Initialize();
		MEngineComponentManager::Initialize();
		MEngineInternalComponents::Initialize();
		MEngineConsole::Initialize();
		MEngineInput::Initialize();
		MEngineText::Initialize();
		MEngineConfig::Initialize();
		MEngineSystemManager::Initialize();
	}

	void Stop()
	{
		MEngineSystemManager::Shutdown();
		MEngineConfig::Shutdown();
		MEngineText::Shutdown();
		MEngineInput::Shutdown();
		MEngineConsole::shutdown();
		MEngineInternalComponents::Shutdown();
		MEngineComponentManager::Shutdown();
		MEngineEntityManager::Shutdown();
		MEngineUtility::Shutdown();
		MEngineGraphics::Shutdown(); // TODODB: Place this where it should be after the initialize has been moved in to Start()
	}

	void Update()
	{
		MEngineUtility::Update();
		MEngineInput::Update();
		MEngineConsole::Update();
	}
}