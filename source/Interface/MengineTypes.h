#pragma once
#include <MUtilityBitset.h>
#include <MUtilityStrongID.h>
#include <MUtilityTypes.h>

#define MENGINE_INVALID_COMPONENT_MASK MUTILITY_INVALID_BITMASK_ID

namespace MEngine
{
	typedef MUtilityBitmaskID ComponentMask;

	struct EntityIDTag {};
	typedef MUtility::StrongID<EntityIDTag, int32_t, -1>	EntityID;

	struct TextureIDTag {};
	typedef MUtility::StrongID<TextureIDTag, int32_t, -1>	TextureID;

	struct SystemIDTag {};
	typedef MUtility::StrongID<SystemIDTag, int32_t, -1>	SystemID;

	struct GameModeIDTag {};
	typedef MUtility::StrongID<GameModeIDTag, int32_t, -1>	GameModeID;

	struct FontIDTag {};
	typedef MUtility::StrongID<FontIDTag, int32_t, -1>		FontID;

	struct CommandIDTag {};
	typedef MUtility::StrongID<CommandIDTag, int32_t, -1>	CommandID;

	enum class InitFlags : MUtility::BitSet
	{
		StartWindowCentered = 1 << 0, // Will override WindowPosX and WindowPosY parameters
		RememberWindowPosition = 1 << 1, // Will override WindowPosX, WindowPosY and StartWindowCentered if there are no config values set for window position

		None,
	};
	CREATE_BITFLAG_OPERATOR_SIGNATURES(InitFlags);

	enum class TextAlignment
	{
		TopLeft,
		TopCentered,
		TopRight,
		CenterLeft,
		CenterCentered,
		CenterRight,
		BottomLeft,
		BottomCentered,
		BottomRight
	};
}