#pragma once
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