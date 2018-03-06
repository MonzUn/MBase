#pragma once
#include "interface/mengineUtility.h"
#include "mengineComponentManagerInternal.h"
#include "mengineConfigInternal.h"
#include "mengineConsoleInternal.h"
#include "mengineEntityManagerInternal.h"
#include "mengineGraphicsInternal.h"
#include "mengineInternalComponentsInternal.h"
#include "mengineInputInternal.h"
#include "mengineSystemManagerInternal.h"
#include "mengineTextInternal.h"
#include "mengineUtilityInternal.h"

namespace MEngineGlobalSystems
{
	void Start()
	void Start() // TODODB: Check if any systm can fail to initialize and return bool accordingly (log inside each system's Initialize() function)
	{
		MEngineUtility::Initialize();
		MEngineConsole::Initialize();
		MEngineEntityManager::Initialize();
		MEngineComponentManager::Initialize();
		MEngineInternalComponents::Initialize();
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
		MEngineInternalComponents::Shutdown();
		MEngineComponentManager::Shutdown();
		MEngineEntityManager::Shutdown();
		MEngineGraphics::Shutdown();
		MEngineConsole::shutdown();
		MEngineUtility::Shutdown();
	}

	void Update()
	{
		MEngineUtility::Update();
		MEngineInput::Update();
	}
}