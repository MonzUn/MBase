#pragma once
#include <string>

namespace MEngineConfig
{
	int64_t		GetInt(const std::string& key, int64_t defaultValue = -1);
	double		GetDouble(const std::string& key, double defaultValue = -1.0f);
	bool		GetBool(const std::string& key, bool defaultValue = false);
	std::string GetString(const std::string& key, const std::string& defaultValue = "");

	void SetInt(std::string key, int64_t value);
	void SetDecimal(std::string key, double value);
	void SetBool(std::string key, bool value);
	void SetString(std::string key, const std::string& value);

	void WriteConfigFile();
	void ReadConfigFile();

	void SetConfigFilePath(const std::string& relativeFilePathAndName);
}