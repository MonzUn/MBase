#include "interface/mengineSystem.h"
#include "mengineSystemInternal.h"
#include "frameCounter.h"
#include <set>

namespace std
{
	template <>
	struct less<MEngineSystem::System*>
	{
		bool operator() (const MEngineSystem::System* lhs, const MEngineSystem::System* rhs)
		{
			return lhs->GetPriority() < rhs->GetPriority();
		}
	};
}

namespace MEngineSystem
{
	std::set<MEngineSystem::System*>*	m_Systems;
	FrameCounter						m_PresentationFrameCounter;
	FrameCounter						m_SimulationFrameCounter;
	float								m_AccumulatedSimulationTime	= 0.0f;
	float								m_SimulationSpeed			= MEngineSystem::DEFAULT_SIMULATION_SPEED;
	float								m_SimulationTimeStep		= MEngineSystem::DEFAULT_TIME_STEP;
}

// ---------- INTERFACE ----------

void MEngineSystem::RegisterSystem(System* system)
{
	m_Systems->insert(system);
	system->Initialize();
}

// ---------- INTERNAL ----------

void MEngineSystem::Initialize()
{
	m_Systems = new std::set<MEngineSystem::System*>();
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