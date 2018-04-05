#pragma once
#include "MEngineTypes.h"
#include <MUtilityBitset.h>
#include <stdint.h>

namespace MEngine
{
	enum class SystemSettings : MUtility::BitSet
	{
		NONE = 0,
		NO_TRANSITION_RESET = 1 << 0,
	};
	CREATE_BITFLAG_OPERATOR_SIGNATURES(SystemSettings);

	class System
	{
	public:
		System(SystemSettings settings = SystemSettings::NONE) : m_SystemSettings(settings) {}
		virtual ~System() {};
		virtual void Initialize() {};
		virtual void Shutdown() { m_IsSuspended = false; };
		virtual void Suspend() { m_IsSuspended = true; };
		virtual void Resume() { m_IsSuspended = false; };

		virtual void UpdatePresentationLayer(float deltaTime) {};
		virtual void UpdateSimulationLayer(float timeStep) {};

		bool IsSuspended() { return m_IsSuspended; }

		SystemID GetID() const { return m_ID; };
		void SetID(SystemID newID) { m_ID = newID; }; // TODODB: Find some way to only allow the engine to set IDs

		SystemSettings GetSystemSettings() const {return m_SystemSettings;}

		private:
			SystemID m_ID = MENGINE_INVALID_SYSTEM_ID;
			SystemSettings m_SystemSettings = SystemSettings::NONE;
			bool m_IsSuspended = false;
	};
}