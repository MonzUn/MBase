#include "frameCounter.h"
#include <SDL_timer.h>

constexpr int32_t	FRAMECOUNTER_SAMPLE_COUNT	= 100;
constexpr int32_t	MILLISECONDS_PER_SECOND		= 1000;

using namespace MEngine;

FrameCounter::FrameCounter()
{
	Reset();
}

void FrameCounter::Tick()
{
	uint64_t performanceCounter = SDL_GetPerformanceCounter();

	m_AverageDeltaTime -= m_AverageDeltaTime / FRAMECOUNTER_SAMPLE_COUNT;
	m_AverageDeltaTime += performanceCounter / FRAMECOUNTER_SAMPLE_COUNT;

	m_LastDeltaTime = static_cast<float>((performanceCounter - m_LastTickTime) / m_PerformaceFrequency);
	m_LastTickTime = performanceCounter;
	++m_FrameCount;
}

void FrameCounter::Reset()
{
	m_FrameCount			= 0;
	m_LastTickTime			= 0;
	m_LastDeltaTime			= 0.0f;
	m_AverageDeltaTime		= 0.0f;
	m_PerformaceFrequency	= SDL_GetPerformanceFrequency();
}

float FrameCounter::GetDeltaTime() const
{
	return m_LastDeltaTime;
}

float FrameCounter::GetAverageDeltaTime() const
{
	return m_AverageDeltaTime;
}

uint32_t FrameCounter::GetFPS() const
{
	return  MILLISECONDS_PER_SECOND / static_cast<uint32_t>(m_LastDeltaTime);
}

uint32_t FrameCounter::GetAverageFPS() const
{
	return MILLISECONDS_PER_SECOND / static_cast<uint32_t>(m_AverageDeltaTime);
}

uint64_t FrameCounter::GetPerformanceFrequency() const
{
	return m_PerformaceFrequency;
}