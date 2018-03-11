#pragma once
#include "MEngineTypes.h"
#include <stdint.h>

namespace MEngine
{
	class System
	{
	public:
		virtual ~System() {};
		virtual void Initialize() {};
		virtual void Shutdown() {};

		virtual void UpdatePresentationLayer(float deltaTime) {};
		virtual void UpdateSimulationLayer(float timeStep) {};

		SystemID GetID() const { return ID; };
		void SetID(SystemID newID) { ID = newID; }; // TODODB: Find some way to only allow the engine to set IDs

		private:
			SystemID ID = INVALID_MENGINE_SYSTEM_ID;
	};
}