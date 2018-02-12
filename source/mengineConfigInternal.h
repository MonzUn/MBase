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

	void Initialize();
	void Shutdown();
}