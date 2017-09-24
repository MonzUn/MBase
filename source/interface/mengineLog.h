#pragma once
#include "mengineMacros.h"
#include <cstdint>
#include <string>
#include <sstream>

#ifndef MENGINE_DISABLE_LOGGING

	#ifdef _DEBUG

		#define MLOG_ERROR(message, category) MEngineLog::InputStream << message, MEngineLog::Log(MEngineLog::InputStream.str(), category, MEngineLogLevel::LOG_ERROR, MEngineLogMode::Debug, __FILE__, MENGINE_STRINGIFY(__LINE__), __func__), MEngineLog::InputStream.clear()
		#define MLOG_WARNING(message, category) MEngineLog::InputStream << message, MEngineLog::Log(MEngineLog::InputStream.str(), category, MEngineLogLevel::LOG_WARNING, MEngineLogMode::Debug, __FILE__, MENGINE_STRINGIFY(__LINE__), __func__), MEngineLog::InputStream.clear()
		#define MLOG_INFO(message, category) MEngineLog::InputStream << message, MEngineLog::Log(MEngineLog::InputStream.str(), category, MEngineLogLevel::LOG_INFO, MEngineLogMode::Debug, __FILE__, MENGINE_STRINGIFY(__LINE__), __func__), MEngineLog::InputStream.clear()
		#define MLOG_DEBUG(message, category) MEngineLog::InputStream << message, MEngineLog::Log(MEngineLog::InputStream.str(), category, MEngineLogLevel::LOG_DEBUG, MEngineLogMode::Debug, __FILE__, MENGINE_STRINGIFY(__LINE__), __func__), MEngineLog::InputStream.clear()

	#else

		#define MLOG_ERROR(message, category) MEngineLog::InputStream << message, MEngineLog::Log(MEngineLog::InputStream.str(), category, MEngineLogLevel::LOG_ERROR)
		#define MLOG_WARNING(message, category) MEngineLog::InputStream << message, MEngineLog::Log(MEngineLog::InputStream.str(), category, MEngineLogLevel::LOG_WARNING)
		#define MLOG_INFO(message, category) MEngineLog::InputStream << message, MEngineLog::Log(MEngineLog::InputStream.str(), category, MEngineLogLevel::LOG_INFO)
		#define MLOG_DEBUG(message, category) MEngineLog::InputStream << message, MEngineLog::Log(MEngineLog::InputStream.str(), category, MEngineLogLevel::LOG_DEBUG)

	#endif

#else

	#define MLOG_ERROR(message, category)
	#define MLOG_WARNING(message, category)
	#define MLOG_INFO(message, category)
	#define MLOG_DEBUG(message, category)

#endif

namespace MEngineLogLevel
{
	enum LogLevel : uint8_t
	{
		LOG_ERROR	= 0x01,
		LOG_WARNING = 0x02,
		LOG_INFO	= 0x04,
		LOG_DEBUG	= 0x08,

		// Not a valid log types. Only allowed when registering interest. 
		NONE		= 0x00,
		ALL			= 0xFF, 
	};
}

enum class MEngineLogMode
{
	Normal,
	Debug
};

namespace MEngineLog
{
#ifndef MENGINE_DISABLE_LOGGING
	static std::stringstream InputStream; // Do not use externally! Only used for MEngine logging macros
#endif

	void SetInterest(const std::string& category, MEngineLogLevel::LogLevel newInterestLevels);
	void Log(const std::string& message, const std::string& category, MEngineLogLevel::LogLevel logLevel, MEngineLogMode logMode = MEngineLogMode::Normal, const char* file = nullptr, const char* line = nullptr, const char* functionName = nullptr);
	void FlushToDisk();
}