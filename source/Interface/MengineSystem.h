#pragma once
#include "MEngineTypes.h"
#include <stdint.h>

namespace MEngine
{
	enum SystemSettings : uint32_t // TODODB: Make enum class for type safety
	{
		NONE = 0,
		NO_TRANSITION_RESET = 1 << 0,
	};

	class System
	{
	public:
		System(SystemSettings settings = SystemSettings::NONE) : m_SystemSettings(settings) {}
		virtual ~System() {};
		virtual void Initialize() {}; // TODODB: Add checks in debug mode to check so that the system is not initialized or shutdown in the wrong state
		virtual void Shutdown() { m_IsSuspended = false; };
		virtual void Suspend() { m_IsSuspended = true; };
		virtual void Resume() { m_IsSuspended = false; };

		virtual void UpdatePresentationLayer(float deltaTime) {};
		virtual void UpdateSimulationLayer(float timeStep) {};

		bool IsSuspended() {return m_IsSuspended;}

		SystemID GetID() const { return ID; };
		void SetID(SystemID newID) { ID = newID; }; // TODODB: Find some way to only allow the engine to set IDs

		SystemSettings GetSystemSettings() const {return m_SystemSettings;}

		private:
			SystemID ID = INVALID_MENGINE_SYSTEM_ID; // TODODB: Rename using m_
			SystemSettings m_SystemSettings = SystemSettings::NONE;
			bool m_IsSuspended = false;
	};
}