#pragma once
#include <stdint.h>

class FrameCounter
{
public:
	FrameCounter();

	void		Tick();
	void		Reset();

	float		GetDeltaTime() const;
	float		GetAverageDeltaTime() const;
	uint32_t	GetFPS() const;
	uint32_t	GetAverageFPS() const;
	uint64_t	GetPerformanceFrequency() const;

private:
	uint64_t	m_FrameCount;
	uint64_t	m_LastTickTime;
	float		m_LastDeltaTime;
	float		m_AverageDeltaTime;
	uint64_t	m_PerformaceFrequency;
};