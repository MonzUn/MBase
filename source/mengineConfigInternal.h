#pragma once
#include <string>

namespace MEngineConfig
{
	namespace ValueType
	{
		enum ValueType : int32_t
		{
			INTEGER,
			DECIMAL,
			BOOLEAN,
			STRING,

			INVALID
		};
	}

	struct ConfigEntry
	{
		union ValueContainer
		{
			ValueContainer(int64_t intValue) : IntegerValue(intValue) {}
			ValueContainer(double decimalValue) : DecimalValue(decimalValue) {}
			ValueContainer(bool booleanValue) : BooleanValue(booleanValue) {}
			ValueContainer(char* stringValue) : StringValue(stringValue) {}

			int64_t IntegerValue;
			double	DecimalValue;
			bool	BooleanValue;
			char*	StringValue;
		};

		ConfigEntry(ValueType::ValueType type, ValueContainer value) : Type(type), Value(value) {}

		~ConfigEntry()
		{
			if (Type == ValueType::STRING)
				free(Value.StringValue);
		}

		const ValueType::ValueType Type = ValueType::INVALID;
		ValueContainer Value;
	};

	const std::string CONFIG_EXTENSION = ".cfg";
	const std::string DEFAULT_CONFIG_FILE_NAME = "config";
	const std::string DEFAULT_CONFIG_FOLDER_NAME = "config";
	const std::string DEFAULT_CONFIG_FILE_RELATIVE_PATH = DEFAULT_CONFIG_FOLDER_NAME + '/' + DEFAULT_CONFIG_FILE_NAME;

	void Initialize();
	void Shutdown();
}