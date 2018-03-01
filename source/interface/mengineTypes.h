#pragma once
#include <MUtilityTypes.h>

#define INVALID_MENGINE_COMPONENT_MASK MUTILITY_INVALID_BITMASK_ID
#define INVALID_MENGINE_ENTITY_ID -1
#define INVALID_MENGINE_TEXTURE_ID -1
#define INVALID_MENGINE_SYSTEM_ID -1
#define INVALID_MENGINE_GAME_MODE_ID -1

namespace MEngine
{
	typedef MUtilityBitmaskID ComponentMask;
	typedef MUtilityID	EntityID;
	typedef MUtilityID	TextureID;
	typedef MUtilityID	SystemID;
	typedef MUtilityID	GameModeID;
}