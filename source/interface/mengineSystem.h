#pragma once
#include <stdint.h>

namespace MEngineSystem
{
	constexpr float MENGINE_TIME_STEP_FPS_120	= 0.00833333f;
	constexpr float MENGINE_TIME_STEP_FPS_60	= 0.01666667f;
	constexpr float MENGINE_TIME_STEP_FPS_30	= 0.03333333f;
	constexpr float MENGINE_TIME_STEP_FPS_15	= 0.06666666f;

	class System
	{
	public:
		System(uint32_t priority) { m_Priority = priority; }
		virtual ~System() {};
		virtual void Initialize() {};
		virtual void Shutdown() {};

		virtual void UpdatePresentationLayer(float deltaTime) {};
		virtual void UpdateSimulationLayer(float timeStep) {};

		uint32_t GetPriority() const { return m_Priority; }

	protected:
		uint32_t m_Priority = 0;
	};

	void RegisterSystem(System* system);
}