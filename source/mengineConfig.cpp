#include "interface/mengineConfig.h"
#include "mengineConfigInternal.h"
#include "interface/mengineUtility.h"
#include <MUtilityFile.h>
#include <MUtilityLog.h>
#include <MUtilityString.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_map>

#define LOG_CATEGORY_CONFIG "MEngineConfig"

namespace MEngineConfig
{
	const std::string* CONFIG_EXTENSION;
	const std::string* DEFAULT_CONFIG_FILE_NAME;
	const std::string* DEFAULT_CONFIG_FOLDER_NAME;
	const std::string* DEFAULT_CONFIG_FILE_RELATIVE_PATH;

	std::string* m_ConfigFilePath;
	std::string* m_ConfigDirectoryPath;
	std::unordered_map<std::string, ConfigEntry*>* m_Entries;
}

using namespace MEngine;
using namespace MEngine::Config;
using namespace MEngineConfig;

// ---------- INTERFACE ----------

int64_t Config::GetInt(const std::string& key, int64_t defaultValue)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = m_Entries->find(keyCopy);
	if (iterator == m_Entries->end())
	{
		iterator = m_Entries->emplace(keyCopy, new ConfigEntry(ValueType::INTEGER, defaultValue)).first;
	}
	return iterator->second->Value.IntegerValue;
}

double Config::GetDouble(const std::string& key, double defaultValue)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = m_Entries->find(keyCopy);
	if (iterator == m_Entries->end())
	{
		iterator = m_Entries->emplace(keyCopy, new ConfigEntry(ValueType::DECIMAL, defaultValue)).first;
	}
	return iterator->second->Value.DecimalValue;
}

bool Config::GetBool(const std::string& key, bool defaultValue)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = m_Entries->find(keyCopy);
	if (iterator == m_Entries->end())
	{
		iterator = m_Entries->emplace(keyCopy, new ConfigEntry(ValueType::BOOLEAN, defaultValue)).first;
	}
	return iterator->second->Value.BooleanValue;
}

std::string Config::GetString(const std::string& key, const std::string& defaultValue)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = m_Entries->find(keyCopy);
	if (iterator == m_Entries->end())
	{
		char* newString = static_cast<char*>(malloc(defaultValue.size() + 1)); // +1 for null terminator
		strcpy(newString, defaultValue.c_str());
		iterator = m_Entries->emplace(keyCopy, new ConfigEntry(ValueType::STRING, newString)).first;
	}
	return iterator->second->Value.StringValue;
}

void Config::SetInt(const std::string& key, int64_t value)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = m_Entries->find(keyCopy);
	if (iterator != m_Entries->end())
	{
		if (iterator->second->Type == ValueType::INTEGER)
			iterator->second->Value.IntegerValue = value;
		else
			MLOG_WARNING("Attempted to assign non integer value to integer config entry; key = " << keyCopy, LOG_CATEGORY_CONFIG);
	}
	else
		iterator = m_Entries->emplace(keyCopy, new ConfigEntry(ValueType::INTEGER, value)).first;
}

void Config::SetDecimal(const std::string& key, double value)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = m_Entries->find(keyCopy);
	if (iterator != m_Entries->end())
	{
		if (iterator->second->Type == ValueType::DECIMAL)
			iterator->second->Value.DecimalValue = value;
		else
			MLOG_WARNING("Attempted to assign non decimal value to decimal config entry; key = " << keyCopy, LOG_CATEGORY_CONFIG);
	}
	else
		iterator = m_Entries->emplace(key, new ConfigEntry(ValueType::DECIMAL, value)).first;
		
}

void Config::SetBool(const std::string& key, bool value)
{
	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = m_Entries->find(keyCopy);
	if (iterator != m_Entries->end())
	{
		if (iterator->second->Type == ValueType::BOOLEAN)
			iterator->second->Value.BooleanValue = value;
		else
			MLOG_WARNING("Attempted to assign non boolean value to boolean config entry; key = " << keyCopy, LOG_CATEGORY_CONFIG);
	}
	else
		iterator = m_Entries->emplace(keyCopy, new ConfigEntry(ValueType::BOOLEAN, value)).first;
}

void Config::SetString(const std::string& key, const std::string& value)
{
	char* newString = static_cast<char*>(malloc(key.size() + 1)); // +1 for null terminator
	strcpy(newString, value.c_str());

	std::string keyCopy = key;
	std::transform(keyCopy.begin(), keyCopy.end(), keyCopy.begin(), ::tolower);
	auto iterator = m_Entries->find(keyCopy);
	if (iterator != m_Entries->end())
	{
		if (iterator->second->Type == ValueType::STRING)
		{
			free(iterator->second->Value.StringValue);
			iterator->second->Value.StringValue = newString;
		}
		else
			MLOG_WARNING("Attempted to assign non string value to string config entry; key = " << keyCopy, LOG_CATEGORY_CONFIG);
	}
	else
		iterator = m_Entries->emplace(keyCopy, new ConfigEntry(ValueType::STRING, newString)).first;
}

void Config::WriteConfigFile()
{
	std::stringstream stringStream;
	for (auto& keyAndEntry : *m_Entries)
	{
		stringStream << keyAndEntry.first << " = ";

		const ConfigEntry::ValueContainer& value = keyAndEntry.second->Value;
		switch (keyAndEntry.second->Type)
		{
			case ValueType::INTEGER:
			{
				stringStream << value.IntegerValue;
			} break;

			case ValueType::DECIMAL:
			{
				stringStream << value.DecimalValue;
				if (value.DecimalValue == std::floor(value.DecimalValue)) // If the value is without decimal part; write a decimal so that the parser handles it as a decimal value and not an integer value
					stringStream << ".0";
			} break;

			case ValueType::BOOLEAN:
			{
				stringStream << value.BooleanValue ? "true" : "false";
			} break;

			case ValueType::STRING:
			{
				stringStream << "\"" << value.StringValue << "\"";
			} break;

			case ValueType::INVALID:
			{
				MLOG_WARNING("Config entries contained entry with invalid type", LOG_CATEGORY_CONFIG);
			} break;
			
		default:
			MLOG_WARNING("Config entries contained entry with unknown type", LOG_CATEGORY_CONFIG);
			break;
		}

		stringStream << std::endl;
	}
	
	if (!MUtility::DirectoryExists(m_ConfigDirectoryPath->c_str()))
		MUtility::CreateDir(m_ConfigDirectoryPath->c_str());

	std::ofstream outstream = std::ofstream(*m_ConfigFilePath, std::ofstream::out | std::ofstream::trunc);
	if (outstream.is_open())
	{
		outstream << stringStream.str();
		outstream.close();
	}
}

void Config::ReadConfigFile()
{
	if (!MUtility::DirectoryExists(m_ConfigDirectoryPath->c_str()))
	{
		MLOG_WARNING("Config file directory does not exist; Path = " << m_ConfigDirectoryPath, LOG_CATEGORY_CONFIG);
		return;
	}

	if (!MUtility::FileExists(m_ConfigFilePath->c_str()))
	{
		MLOG_WARNING("Config file does not exist; Path = " << m_ConfigFilePath, LOG_CATEGORY_CONFIG);
		return;
	}

	std::stringstream stringStream;
	stringStream << MUtility::GetFileContentAsString(*m_ConfigFilePath);

	while (stringStream.good())
	{
		std::string line, key, value = "";
		getline(stringStream, line);
		if (line == "")
			continue;

		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end()); // Strip whitespaces
		
		size_t dividerPos = line.find("=");
		if (dividerPos == 0)
		{
			MLOG_WARNING("Found config line with missing key; line = " << line, LOG_CATEGORY_CONFIG);
			continue;
		}
		if (dividerPos == std::string::npos)
		{
			MLOG_WARNING("Found config line without divider; line = " << line, LOG_CATEGORY_CONFIG);
			continue;
		}
		if (dividerPos == line.length() - 1)
		{
			MLOG_WARNING("Found config line without key; line = " << line, LOG_CATEGORY_CONFIG);
			continue;
		}

		key = line.substr(0, dividerPos);
		value = line.substr(dividerPos + 1, line.length() - dividerPos);
		
		ValueType::ValueType type = ValueType::INVALID;
		if (value[0] == '\"' || value[value.size() - 1] == '\"')
		{
			if (value[0] != '\"')
			{
				MLOG_WARNING("Found config string missing initial \" character; line = " << line, LOG_CATEGORY_CONFIG);
				continue;
			}
			else if (value[value.size() - 1] != '\"')
			{
				MLOG_WARNING("Found config string missing ending \" character; line = " << line, LOG_CATEGORY_CONFIG);
				continue;
			}

			value.erase(std::remove(value.begin(), value.end(), '\"'), value.end()); // Remove the "" from the string
			char* stringValue = static_cast<char*>(malloc(value.size() + 1)); // +1 for the null terminator
			strcpy(stringValue, value.c_str());
			m_Entries->emplace(key, new ConfigEntry(ValueType::STRING, stringValue));
		}
		else if (MUtilityString::IsStringNumber(value))
		{
			int64_t intValue = strtoll(value.c_str(), nullptr, 0);
			m_Entries->emplace(key, new ConfigEntry(ValueType::INTEGER, intValue));
		}
		else if (value == "true" || value == "false")
		{
			bool boolValue = (value == "true");
			m_Entries->emplace(key, new ConfigEntry(ValueType::BOOLEAN, boolValue));
		}
		else if (MUtilityString::IsStringNumberExcept(value, '.', 1) || MUtilityString::IsStringNumberExcept(value, ',', 1))
		{
			double doubleValue = std::stod(value);
			m_Entries->emplace(key, new ConfigEntry(ValueType::DECIMAL, doubleValue));
		}
		else
			MLOG_WARNING("Unable to determine value type of config line; line = " << line, LOG_CATEGORY_CONFIG);
	}
}

void Config::SetConfigFilePath(const std::string& relativeFilePathAndName)
{
	*m_ConfigFilePath		= MEngine::GetExecutablePath() + '/' + relativeFilePathAndName + *CONFIG_EXTENSION;
	*m_ConfigDirectoryPath	= MUtility::GetDirectoryPathFromFilePath(*m_ConfigFilePath);
}

// ---------- INTERNAL ----------

void MEngineConfig::Initialize()
{
	CONFIG_EXTENSION = new std::string(".cfg");
	DEFAULT_CONFIG_FILE_NAME = new std::string("config");
	DEFAULT_CONFIG_FOLDER_NAME = new std::string("config");
	DEFAULT_CONFIG_FILE_RELATIVE_PATH = new std::string(*DEFAULT_CONFIG_FOLDER_NAME + '/' + *DEFAULT_CONFIG_FILE_NAME);

	m_ConfigFilePath = new std::string("NOT_SET");
	m_ConfigDirectoryPath = new std::string("NOT_SET");
	m_Entries = new std::unordered_map<std::string, ConfigEntry*>();

	SetConfigFilePath(*DEFAULT_CONFIG_FILE_RELATIVE_PATH);
	ReadConfigFile();
}

void MEngineConfig::Shutdown()
{
	WriteConfigFile();

	for (auto& keyAndValue : *m_Entries)
	{
		delete keyAndValue.second;
	}
	m_Entries->clear();

	delete CONFIG_EXTENSION;
	delete DEFAULT_CONFIG_FILE_NAME;
	delete DEFAULT_CONFIG_FOLDER_NAME;
	delete DEFAULT_CONFIG_FILE_RELATIVE_PATH;

	delete m_ConfigFilePath;
	delete m_ConfigDirectoryPath;
	delete m_Entries;
}