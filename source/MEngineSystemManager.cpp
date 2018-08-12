#include "Interface/MEngineSystem.h"
#include "Interface/MengineConsole.h"
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
void HandleSuspendResumeRequests();
void RegisterInternalSystem(MEngine::System* system, uint32_t priority);
void RegisterInternalSystems();

bool CompareSystemPriorities(const std::pair<MEngine::SystemID, uint32_t>& lhs, const std::pair<MEngine::SystemID, uint32_t>& rhs)
{
	return lhs.second < rhs.second;
};

// TODODB: Make sure that the external application can not manipulate(Add, remove, suspend, resume etc) internal systems

using namespace MEngine;
using namespace MEngineSystemManager;

namespace MEngineSystemManager
{
	GameModeList*							m_GameModes;
	MUtility::MUtilityIDBank<GameModeID>*	m_GameModeIDBank;
	MEngine::GameModeID						m_ActiveGameModeID;
	MEngine::GameModeID						m_RequestedGameModeID;

	std::vector<MEngine::System*>*		m_Systems;
	MUtility::MUtilityIDBank<SystemID>*	m_SystemIDBank;

	std::vector<std::pair<SystemID, bool>>* m_SuspendResumeRequests;

	std::vector<SystemID>* m_InternalSystemList;
	std::vector<uint32_t>* m_InternalSystemPriorities;

	MEngine::FrameCounter	m_PresentationFrameCounter;
	MEngine::FrameCounter	m_SimulationFrameCounter;
	float					m_AccumulatedSimulationTime	= 0.0f;
	float					m_SimulationSpeed			= DEFAULT_SIMULATION_SPEED;
	float					m_SimulationTimeStep		= DEFAULT_TIME_STEP;
}

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
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (!m_SystemIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to unregister system using an invalid system ID; ID = " << ID, LOG_CATEGORY_SYSTEM_MANAGER);
		return false;
	}
#endif

	bool result = false;
	for (int i = 0; i < m_Systems->size(); ++i)
	{
		if ((*m_Systems)[i]->GetID() == ID)
		{
			UnregisterSystemCommands(ID);
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

void MEngine::RequestSuspendSystem(SystemID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (!m_SystemIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to suspend system using an invalid systemID; ID = " << ID, LOG_CATEGORY_SYSTEM_MANAGER);
		return;
	}

	if ((*m_Systems)[ID]->IsSuspended())
	{
		MLOG_WARNING("Attempted to suspend already suspended system; ID = " << ID, LOG_CATEGORY_SYSTEM_MANAGER);
		return;
	}
#endif
	m_SuspendResumeRequests->emplace_back(std::make_pair(ID, true));
}

void MEngine::RequestResumeSystem(SystemID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (!m_SystemIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to resume system using an invalid systemID; ID = " << ID, LOG_CATEGORY_SYSTEM_MANAGER);
		return;
	}

	if (!(*m_Systems)[ID]->IsSuspended())
	{
		MLOG_WARNING("Attempted to resume non suspended system; ID = " << ID, LOG_CATEGORY_SYSTEM_MANAGER);
		return;
	}
#endif

	m_SuspendResumeRequests->emplace_back(std::make_pair(ID, false));
}

GameModeID MEngine::CreateGameMode()
{
	GameModeID gameModeID = m_GameModeIDBank->GetID();
	if (m_GameModeIDBank->IsIDLast(gameModeID))
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

// TODODB: Make it possible for a system to start suspended or not suspended in different game modes
bool MEngine::AddSystemToGameMode(GameModeID gameModeID, SystemID systemID, uint32_t priority) // TODODB: Add shutdown and startup priorities (could be implemented as a priorities struct that is used to sort systems before shutdown and startup)
{
	uint32_t shiftedPriority = priority + MENGINE_MIN_SYSTEM_PRIORITY;
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (!m_SystemIDBank->IsIDActive(systemID))
	{
		MLOG_WARNING("Attempted to add system to game mode using an invalid SystemID; Game mode ID = " << gameModeID << "; system ID = " << systemID, LOG_CATEGORY_SYSTEM_MANAGER);
		return false;
	}

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
	if (m_ActiveGameModeID == newGameModeID)
	{
		MLOG_WARNING("Attempted to change to the already active game mode; game mode ID = " << newGameModeID, LOG_CATEGORY_SYSTEM_MANAGER);
		return false;
	}

	if (m_RequestedGameModeID.IsValid() && Settings::HighLogLevel)
		MLOG_WARNING("Requested game mode change before the last request could be fulfilled; game mode with ID " << m_RequestedGameModeID << "will be skipped and game mode with ID " << newGameModeID << " will be used instead", LOG_CATEGORY_SYSTEM_MANAGER);

	m_RequestedGameModeID = newGameModeID;
	return true;
}

bool MEngine::IsSystemIDValid(SystemID ID)
{
	return m_SystemIDBank->IsIDActive(ID);
}

bool MEngine::IsGameModeIDValid(GameModeID ID)
{
	return m_GameModeIDBank->IsIDActive(ID);
}

bool MEngine::IsSystemInGameMode(SystemID systemID, GameModeID gameModeID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (!m_SystemIDBank->IsIDActive(systemID))
	{
		MLOG_WARNING("Attempted to check if system is in gamemode using an invalid system ID; ID = " << systemID, LOG_CATEGORY_SYSTEM_MANAGER);
		return false;
	}

	if (!m_GameModeIDBank->IsIDActive(gameModeID))
	{
		MLOG_WARNING("Attempted to check if system is in gamemode using an invalid gamemode ID; ID = " << gameModeID, LOG_CATEGORY_SYSTEM_MANAGER);
		return false;
	}
#endif

	bool result = false;
	for (int k = 0; k < m_GameModes[gameModeID].size(); ++k)
	{
		if ((*m_GameModes)[gameModeID][k].first == systemID)
		{
			result = true;
			break;
		}
	}

	return result;
}

// ---------- INTERNAL ----------

void MEngineSystemManager::Initialize()
{
	m_GameModes					= new GameModeList();
	m_GameModeIDBank			= new MUtility::MUtilityIDBank<GameModeID>;
	m_Systems					= new std::vector<System*>();
	m_SystemIDBank				= new MUtility::MUtilityIDBank<SystemID>;
	m_SuspendResumeRequests		= new std::vector<std::pair<SystemID, bool>>();
	m_InternalSystemList		= new std::vector<SystemID>();
	m_InternalSystemPriorities	= new std::vector<uint32_t>();

	RegisterInternalSystems();
}

void MEngineSystemManager::Shutdown()
{
	// Shut down the currently active systems
	const GameModeSystemList& gameModeSystems = (*m_GameModes)[m_ActiveGameModeID];
	for (int i = 0; i < gameModeSystems.size(); ++i)
	{
		(*m_Systems)[gameModeSystems[i].first]->Shutdown();
	}

	delete m_GameModes;
	delete m_GameModeIDBank;

	for (int i = 0; i < m_InternalSystemList->size(); ++i)
	{
		(*m_Systems)[(*m_InternalSystemList)[i]]->Shutdown();
	}
	delete m_InternalSystemList;
	delete m_InternalSystemPriorities;

	for (auto& system : *m_Systems)
	{
		delete system;
	}
	m_Systems->clear();
	delete m_Systems;
	delete m_SystemIDBank;

	delete m_SuspendResumeRequests;
}

void MEngineSystemManager::Update()
{
	HandleSuspendResumeRequests();

	// Change game mode if requested
	if (m_RequestedGameModeID.IsValid()) // A valid ID indicates that a request has been made
		ChangeToRequestedGameMode();

	// Update systems
	m_PresentationFrameCounter.Tick();
	float deltaTime = m_PresentationFrameCounter.GetDeltaTime();
	const GameModeSystemList& activeSystems = (*m_GameModes)[m_ActiveGameModeID];
	for(int i = 0; i < activeSystems.size(); ++i)
	{
		System* system = (*m_Systems)[activeSystems[i].first];
		if(!system->IsSuspended())
			system->UpdatePresentationLayer(deltaTime);
	}

	m_AccumulatedSimulationTime += deltaTime;
	if (m_AccumulatedSimulationTime > m_SimulationSpeed)
	{
		m_SimulationFrameCounter.Tick();
		m_AccumulatedSimulationTime -= m_SimulationSpeed;

		for (int i = 0; i < activeSystems.size(); ++i)
		{
			System* system = (*m_Systems)[activeSystems[i].first];
			if (!system->IsSuspended())
				system->UpdateSimulationLayer(m_SimulationTimeStep);
		}
	}
}

// ---------- LOCAL ----------

void ChangeToRequestedGameMode()
{
	// Stop all running systems
	if (m_ActiveGameModeID.IsValid())
	{
		const GameModeSystemList& currentSystems = (*m_GameModes)[m_ActiveGameModeID];
		if (m_ActiveGameModeID.IsValid())
		{
			for (int i = 0; i < currentSystems.size(); ++i)
			{
				System* system = (*m_Systems)[currentSystems[i].first];
				if ((system->GetSystemSettings() & SystemSettings::NO_TRANSITION_RESET) == 0 || !IsSystemInGameMode(currentSystems[i].first, m_RequestedGameModeID))
				{
					UnregisterSystemCommands(system->GetID());
					system->Shutdown();
				}
			}
		}
	}

	UnregisterGameModeCommands(m_ActiveGameModeID);

	// Start systems for the new game mode
	const GameModeSystemList& newSystems = (*m_GameModes)[m_RequestedGameModeID];
	for (int i = 0; i < newSystems.size(); ++i)
	{
		System* system = (*m_Systems)[newSystems[i].first];
		if ((system->GetSystemSettings() & SystemSettings::NO_TRANSITION_RESET) == 0 || !IsSystemInGameMode(newSystems[i].first, m_RequestedGameModeID))
			system->Initialize();
	}
	m_ActiveGameModeID = m_RequestedGameModeID;
	m_RequestedGameModeID.Invalidate();
}

void HandleSuspendResumeRequests() // TODODB: Handle removal and readding of commands on suspend/resume
{
	for (int i = 0; i < m_SuspendResumeRequests->size(); ++i)
	{
		System* system = (*m_Systems)[(*m_SuspendResumeRequests)[i].first];
		(*m_SuspendResumeRequests)[i].second ? system->Suspend() : system->Resume();
	}
	m_SuspendResumeRequests->clear();
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