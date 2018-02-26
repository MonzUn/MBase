#pragma once
#include <string>

namespace MEngine
{
	namespace Config
	{
		int64_t		GetInt(const std::string& key, int64_t defaultValue = -1);
		double		GetDouble(const std::string& key, double defaultValue = -1.0f);
		bool		GetBool(const std::string& key, bool defaultValue = false);
		std::string GetString(const std::string& key, const std::string& defaultValue = "");

		void		SetInt(const std::string& key, int64_t value);
		void		SetDecimal(const std::string& key, double value);
		void		SetBool(const std::string& key, bool value);
		void		SetString(const std::string& key, const std::string& value);

		void		WriteConfigFile();
		void		ReadConfigFile();

		void		SetConfigFilePath(const std::string& relativeFilePathAndName);
	}
}