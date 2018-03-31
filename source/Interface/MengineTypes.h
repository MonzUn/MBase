#pragma once
#include <MUtilityBitset.h>
#include <MUtilityTypes.h>

#define MENGINE_INVALID_COMPONENT_MASK MUTILITY_INVALID_BITMASK_ID
#define MENGINE_INVALID_ENTITY_ID -1
#define MENGINE_INVALID_TEXTURE_ID -1
#define MENGINE_INVALID_SYSTEM_ID -1
#define MENGINE_INVALID_GAME_MODE_ID -1
#define MENGINE_INVALID_FONT_ID -1

namespace MEngine
{
	typedef MUtilityBitmaskID	ComponentMask;
	typedef MUtilityID			EntityID;
	typedef MUtilityID			TextureID;
	typedef MUtilityID			SystemID;
	typedef MUtilityID			GameModeID;
	typedef MUtilityID			FontID;

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