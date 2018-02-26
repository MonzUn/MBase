#include "interface/mengineSystem.h"
#include "mengineSystemInternal.h"
#include "frameCounter.h"
#include <set>

namespace std
{
	template <>
	struct less<MEngine::System*>
	{
		bool operator() (const MEngine::System* lhs, const MEngine::System* rhs)
		{
			return lhs->GetPriority() < rhs->GetPriority();
		}
	};
}

namespace MEngineSystem
{
	std::set<MEngine::System*>*	m_Systems;
	MEngine::FrameCounter		m_PresentationFrameCounter;
	MEngine::FrameCounter		m_SimulationFrameCounter;
	float						m_AccumulatedSimulationTime	= 0.0f;
	float						m_SimulationSpeed			= DEFAULT_SIMULATION_SPEED;
	float						m_SimulationTimeStep		= DEFAULT_TIME_STEP;
}

using namespace MEngine;
using namespace MEngineSystem;

// ---------- INTERFACE ----------

void MEngine::RegisterSystem(System* system)
{
	m_Systems->insert(system);
	system->Initialize();
}

// ---------- INTERNAL ----------

void MEngineSystem::Initialize()
{
	m_Systems = new std::set<System*>();
}

void MEngineSystem::Shutdown()
{
	for (auto& system : *m_Systems)
	{
		system->Shutdown();
		delete system;
	}
	m_Systems->clear();

	delete m_Systems;
}

void MEngineSystem::Update()
{
	m_PresentationFrameCounter.Tick();
	float deltaTime = m_PresentationFrameCounter.GetDeltaTime();

	for (auto& system : *m_Systems)
	{
		system->UpdatePresentationLayer(deltaTime);
	}

	m_AccumulatedSimulationTime += deltaTime;
	if (m_AccumulatedSimulationTime > m_SimulationSpeed)
	{
		m_SimulationFrameCounter.Tick();
		m_AccumulatedSimulationTime -= m_SimulationSpeed;

		for (auto& system : *m_Systems)
		{
			system->UpdateSimulationLayer(m_SimulationTimeStep);
		}
	}
}