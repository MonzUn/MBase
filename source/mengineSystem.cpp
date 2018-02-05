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

std::set<MEngineSystem::System*>	Systems;
FrameCounter						PresentationFrameCounter;
FrameCounter						SimulationFrameCounter;
float								AccumulatedSimulationTime	= 0.0f;
float								SimulationSpeed				= MEngineSystem::DEFAULT_SIMULATION_SPEED;
float								SimulationTimeStep			= MEngineSystem::DEFAULT_TIME_STEP;

// ---------- INTERFACE ----------

void MEngineSystem::RegisterSystem(System* system)
{
	Systems.insert(system);
	system->Initialize();
}

// ---------- INTERNAL ----------

void MEngineSystem::Update()
{
	PresentationFrameCounter.Tick();
	float deltaTime = PresentationFrameCounter.GetDeltaTime();

	for (auto& system : Systems)
	{
		system->UpdatePresentationLayer(deltaTime);
	}

	AccumulatedSimulationTime += deltaTime;
	if (AccumulatedSimulationTime > SimulationSpeed)
	{
		SimulationFrameCounter.Tick();
		AccumulatedSimulationTime -= SimulationSpeed;

		for (auto& system : Systems)
		{
			system->UpdateSimulationLayer(SimulationTimeStep);
		}
	}
}

void MEngineSystem::Shutdown()
{
	for (auto& system : Systems)
	{
		system->Shutdown();
		delete system;
	}
	Systems.clear();
}