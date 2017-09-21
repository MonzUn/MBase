#include "interface/mengineLog.h"
#include "mengineLogInternal.h"
#include "utilities/fileUtility.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#define MENGINE_LOG_CATEGORY_LOG "MEngineLog"

namespace
{
	struct MEngineLogValuePair
	{
		MEngineLogValuePair(MEngineLogLevel::LogLevel initialInterestLevels) : InterestLevels(initialInterestLevels) {}

		MEngineLogLevel::LogLevel	InterestLevels;
		std::stringstream			Log;
	};

	std::unordered_map<std::string, MEngineLogValuePair> Logs;
	std::stringstream MainLog;
	std::stringstream FullInterestLog;
}

void MEngineLog::Initialize()
{
	RegisterCategory(MENGINE_LOG_CATEGORY_LOG);
}

void MEngineLog::Shutdown()
{
	FlushToDisk();
}

void MEngineLog::RegisterCategory(const char* categoryName, MEngineLogLevel::LogLevel initialInterestLevels)
{
	if (Logs.find(categoryName) != Logs.end())
	{
		MLOG_WARNING("Attempted to register logger category \"" << std::string(categoryName) + "\" multiple times; call ignored", MENGINE_LOG_CATEGORY_LOG);
		return;
	}

	Logs.emplace(categoryName, initialInterestLevels);
}

void MEngineLog::Log(const std::string& message, const std::string& category, MEngineLogLevel::LogLevel logLevel, MEngineLogMode logMode, const char* file, const char* line, const char* functionName)
{
	const auto iterator = Logs.find(category);
	if (iterator == Logs.end())
	{
		MLOG_WARNING("Unregistered logging category \"" << category << "\" supplied; call ignored", MENGINE_LOG_CATEGORY_LOG);
		return;
	}

	std::stringstream localStream;

	switch (logLevel)
	{
	case MEngineLogLevel::LOG_ERROR:
		localStream << "Error\n";
		break;

	case MEngineLogLevel::LOG_WARNING:
		localStream << "Warning\n";
		break;

	case MEngineLogLevel::LOG_INFO:
		localStream << "Info\n";
		break;

	case MEngineLogLevel::LOG_DEBUG:
		localStream << "Debug\n";
		break;

	default:
		MLOG_WARNING("Invalid loglevel supplied; call ignored", MENGINE_LOG_CATEGORY_LOG);
		return;
	}

	switch (logMode)
	{
	case MEngineLogMode::Normal:
		localStream << "Category: " << category << '\n' << message << "\n\n";
		break;
	case MEngineLogMode::Debug:
		localStream << "Category: " << category << '\n' << "File: " << file << '\n' << "Line: " << line << '\n' << "Function: " << functionName << "\n - " << message << "\n\n";
		break;
	default:
		MLOG_WARNING("Invalid logMode supplied; call ignored", MENGINE_LOG_CATEGORY_LOG);
		return;
	}

	const std::string fullMessage = localStream.str();

	if (logLevel & iterator->second.InterestLevels)
	{
		iterator->second.Log << fullMessage;
		MainLog << fullMessage;
		std::cout << fullMessage;
	}

	FullInterestLog << fullMessage;
}

void MEngineLog::FlushToDisk()
{
	MEngineFileUtility::CreateFolder("logs");

	std::ofstream outStream;
	outStream.open("logs/mainLog.txt", std::ofstream::out | std::ofstream::trunc);
	if (outStream.is_open())
	{
		outStream << MainLog.rdbuf();
		outStream.close();
	}

	outStream.open("logs/fullInterestLog.txt", std::ofstream::out | std::ofstream::trunc);
	if (outStream.is_open())
	{
		outStream << FullInterestLog.rdbuf();
		outStream.close();
	}

	MEngineFileUtility::CreateFolder("logs/categories");
	for (auto it = Logs.begin(); it != Logs.end(); ++it)
	{
		outStream.open("logs/categories/" + it->first + ".txt", std::ofstream::out | std::ofstream::trunc);
		if (outStream.is_open())
		{
			outStream << it->second.Log.rdbuf();
			outStream.close();
		}
	}
}