#include "Interface/MEngineSystem.h"
#include "Interface/MEngineSettings.h"
#include "MEngineSystemManagerInternal.h"
#include "ButtonSystem.h"
#include "TextBoxSystem.h"
#include "FrameCounter.h"
#include <MUtilityIDBank.h>
#include <algorithm>
#include <set>
#include <vector>

#define LOG_CATEGORY_SYSTEM_MANAGER "MEngineSystemManager"

typedef std::vector<std::pair<MEngine::SystemID, uint32_t>> GameModeSystemList;
typedef std::vector<GameModeSystemList> GameModeList;

void ChangeToRequestedGameMode();
void RegisterInternalSystem(MEngine::System* system, uint32_t priority);
void RegisterInternalSystems();

bool CompareSystemPriorities(const std::pair<MEngine::SystemID, uint32_t>& lhs, const std::pair<MEngine::SystemID, uint32_t>& rhs)
{
	return lhs.second < rhs.second;
};

namespace MEngineSystemManager
{
	GameModeList*					m_GameModes;
	MUtility::MUtilityIDBank*		m_GameModeIDBank;
	MEngine::GameModeID				m_ActiveGameMode	= INVALID_MENGINE_GAME_MODE_ID;
	MEngine::GameModeID				m_RequestedGameMode	= INVALID_MENGINE_GAME_MODE_ID;
	std::vector<MEngine::System*>*	m_Systems;
	MUtility::MUtilityIDBank*		m_SystemIDBank;

	std::vector<MEngine::SystemID>*		m_InternalSystemList;
	std::vector<uint32_t>*				m_InternalSystemPriorities;

	MEngine::FrameCounter			m_PresentationFrameCounter;
	MEngine::FrameCounter			m_SimulationFrameCounter;
	float							m_AccumulatedSimulationTime	= 0.0f;
	float							m_SimulationSpeed			= DEFAULT_SIMULATION_SPEED;
	float							m_SimulationTimeStep		= DEFAULT_TIME_STEP;
}

using namespace MEngine;
using namespace MEngineSystemManager;

// ---------- INTERFACE ----------

SystemID MEngine::RegisterSystem(System* system)
{
	bool isDuplicate = false;
	for (int i = 0; i < m_Systems->size(); ++i)
	{
		if ((*m_Systems)[i] == system) // pointer comparison
		{
			MLOG_WARNING("Attempted to register an already registered system; ID = " << system->GetID(), LOG_CATEGORY_SYSTEM_MANAGER);
			isDuplicate = true;
			break;
		}
	}

	if(!isDuplicate)
	{
		system->SetID(m_SystemIDBank->GetID());
		m_Systems->push_back(system);
	}

	return system->GetID();
}

bool MEngine::UnregisterSystem(SystemID ID)
{
	bool result = false;
	for (int i = 0; i < m_Systems->size(); ++i)
	{
		if ((*m_Systems)[i]->GetID() == ID)
		{
			m_SystemIDBank->ReturnID(ID);
			m_Systems->erase(m_Systems->begin() + i);
			result = true;
			break;
		}
	}

	if(!result)
		MLOG_WARNING("Attempted to unregister a non-registered system; ID = " << ID, LOG_CATEGORY_SYSTEM_MANAGER);

	return result;
}

GameModeID MEngine::CreateGameMode()
{
	GameModeID gameModeID = m_GameModeIDBank->GetID();
	if (gameModeID == m_GameModeIDBank->PeekNextID() - 1)
		m_GameModes->emplace_back(GameModeSystemList());
	else
		m_GameModes->emplace(m_GameModes->begin() + gameModeID, GameModeSystemList());

	for (int i = 0; i < m_InternalSystemList->size(); ++i)
	{
		(*m_GameModes)[gameModeID].emplace_back(std::make_pair((*m_InternalSystemList)[i], (*m_InternalSystemPriorities)[i]));
	}
	std::sort((*m_GameModes)[gameModeID].begin(), (*m_GameModes)[gameModeID].end(), CompareSystemPriorities);

	return gameModeID;
}

bool MEngine::AddSystemToGameMode(GameModeID gameModeID, SystemID systemID, uint32_t priority) // TODODB: Add shutdown and startup priorities (could be implemented as a priorities struct that is sued to sort systems befoer shutdown and startup)
{
	uint32_t shiftedPriority = priority + MENGINE_MIN_SYSTEM_PRIORITY;
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (!m_GameModeIDBank->IsIDActive(gameModeID))
	{
		MLOG_WARNING("Attempted to add system to non existent game mode; Game mode ID = " << gameModeID << "; system ID = " << systemID, LOG_CATEGORY_SYSTEM_MANAGER);
		return false;
	}

	if (priority < MENGINE_MIN_SYSTEM_PRIORITY || priority >= MENGINE_MAX_SYSTEM_PRIORITY)
	{
		MLOG_WARNING("Attempted to add a system using a priorty that is outside of the allowed scope; Min = " << MENGINE_MIN_SYSTEM_PRIORITY << " max = " << MENGINE_MAX_SYSTEM_PRIORITY, LOG_CATEGORY_SYSTEM_MANAGER);
		return false;
	}

	const GameModeSystemList& systems = (*m_GameModes)[gameModeID];
	for (int i = 0; i < systems.size(); ++i)
	{
		if (systems[i].first == systemID && systems[i].second == shiftedPriority)
		{
			MLOG_WARNING("Attempted to add the same system to game mode " << gameModeID <<" more than once using the same priority; system ID = " << systemID << "; priority = " << priority, LOG_CATEGORY_SYSTEM_MANAGER);
			return false;
		}

		if (systems[i].second == shiftedPriority)
		{
			MLOG_WARNING("Priority collision detected when adding system with ID " << systemID << " to game mode with ID " << gameModeID << "; colliding system ID = " << systems[i].first << "; the system will not be added", LOG_CATEGORY_SYSTEM_MANAGER);
			return false;
		}
	}
#endif

	// TODODB: Make sure that it's safe to add game modes to the active game mode while running the updates for the game mode's systems
	(*m_GameModes)[gameModeID].emplace_back(std::make_pair(systemID, shiftedPriority));
	std::sort((*m_GameModes)[gameModeID].begin(), (*m_GameModes)[gameModeID].end(), CompareSystemPriorities);

	return true;
}

bool MEngine::RequestGameModeChange(GameModeID newGameModeID)
{
	if (m_ActiveGameMode == newGameModeID)
	{
		MLOG_WARNING("Attempted to change to the already active game mode; game mode ID = " << newGameModeID, LOG_CATEGORY_SYSTEM_MANAGER);
		return false;
	}

	if (m_RequestedGameMode != INVALID_MENGINE_GAME_MODE_ID && Settings::HighLogLevel)
		MLOG_WARNING("Requested game mode change before the last request could be fulfilled; game mode with ID " << m_RequestedGameMode << "will be skipped and game mode with ID " << newGameModeID << " will be used instead", LOG_CATEGORY_SYSTEM_MANAGER);

	m_RequestedGameMode = newGameModeID;
	return true;
}

// ---------- INTERNAL ----------

void MEngineSystemManager::Initialize()
{
	m_GameModes					= new GameModeList();
	m_GameModeIDBank			= new MUtility::MUtilityIDBank;
	m_Systems					= new std::vector<System*>();
	m_SystemIDBank				= new MUtility::MUtilityIDBank;
	m_InternalSystemList		= new std::vector<SystemID>();
	m_InternalSystemPriorities	= new std::vector<uint32_t>();

	RegisterInternalSystems();
}

void MEngineSystemManager::Shutdown()
{
	// Shut down the currently active systems
	const GameModeSystemList& gameModeSystems = (*m_GameModes)[m_ActiveGameMode];
	for (int i = 0; i < gameModeSystems.size(); ++i)
	{
		(*m_Systems)[gameModeSystems[i].first]->Shutdown();
	}

	delete m_GameModes;
	delete m_GameModeIDBank;

	for (auto& system : *m_Systems)
	{
		delete system;
	}
	m_Systems->clear();
	delete m_Systems;
	delete m_SystemIDBank;

	for(int i = 0; i < m_InternalSystemList->size(); ++i)
	{
		UnregisterSystem((*m_InternalSystemList)[i]);
	}
	delete m_InternalSystemList;
	delete m_InternalSystemPriorities;
}

void MEngineSystemManager::Update()
{
	m_PresentationFrameCounter.Tick();

	// Change game mode if requested
	if (m_RequestedGameMode != INVALID_MENGINE_GAME_MODE_ID)
		ChangeToRequestedGameMode();

	// Update systems
	float deltaTime = m_PresentationFrameCounter.GetDeltaTime();
	const GameModeSystemList& activeSystems = (*m_GameModes)[m_ActiveGameMode];
	for(int i = 0; i < activeSystems.size(); ++i)
	{
		(*m_Systems)[activeSystems[i].first]->UpdatePresentationLayer(deltaTime);
	}

	m_AccumulatedSimulationTime += deltaTime;
	if (m_AccumulatedSimulationTime > m_SimulationSpeed)
	{
		m_SimulationFrameCounter.Tick();
		m_AccumulatedSimulationTime -= m_SimulationSpeed;

		for (int i = 0; i < activeSystems.size(); ++i)
		{
			(*m_Systems)[activeSystems[i].first]->UpdateSimulationLayer(m_SimulationTimeStep);
		}
	}
}

// ---------- LOCAL ----------

void ChangeToRequestedGameMode()
{
	// Stop all running systems
	if (m_ActiveGameMode != INVALID_MENGINE_GAME_MODE_ID)
	{
		const GameModeSystemList& currentSystems = (*m_GameModes)[m_ActiveGameMode];
		if (m_ActiveGameMode != INVALID_MENGINE_GAME_MODE_ID)
		{
			for (int i = 0; i < currentSystems.size(); ++i)
			{
				(*m_Systems)[currentSystems[i].first]->Shutdown();
			}
		}
	}

	// Start systems for the new game mode
	const GameModeSystemList& newSystems = (*m_GameModes)[m_RequestedGameMode];
	for (int i = 0; i < newSystems.size(); ++i)
	{
		(*m_Systems)[newSystems[i].first]->Initialize();
	}
	m_ActiveGameMode = m_RequestedGameMode;
	m_RequestedGameMode = INVALID_MENGINE_GAME_MODE_ID;
}

void RegisterInternalSystem(System* system, uint32_t priority)
{
	SystemID ID = RegisterSystem(system);
	m_InternalSystemList->push_back(ID);
	m_InternalSystemPriorities->push_back(priority);
}

void RegisterInternalSystems() // TODODB: See which internal functionality could be reworked into a system
{
	RegisterInternalSystem(new ButtonSystem(), 0);
	RegisterInternalSystem(new TextBoxSystem(), 1);
}