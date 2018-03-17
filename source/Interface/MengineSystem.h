#pragma once
#include "MEngineTypes.h"
#include <stdint.h>

namespace MEngine
{
	class System
	{
	public:
		virtual ~System() {};
		virtual void Initialize() {}; // TODODB: Add checks in debug mode to check so that the system is not initialized or shutdown in the wrong state
		virtual void Shutdown() {}; // TODODB: Make sure that suspension bool is reset
		virtual void Suspend() { m_IsSuspended = true; };
		virtual void Resume() { m_IsSuspended = false; };

		virtual void UpdatePresentationLayer(float deltaTime) {};
		virtual void UpdateSimulationLayer(float timeStep) {};

		bool IsSuspended() {return m_IsSuspended;}

		SystemID GetID() const { return ID; };
		void SetID(SystemID newID) { ID = newID; }; // TODODB: Find some way to only allow the engine to set IDs

		private:
			SystemID ID = INVALID_MENGINE_SYSTEM_ID; // TODODB: Rename using m_
			bool m_IsSuspended = false;
	};
}