#pragma once
#include "mengineEntityManager.h"
#include "mengineGraphics.h"
#include <stdint.h>

// THIS CLASS IS TEMP UNTIL ENTITY-COMPONENT SYSTEM IS UP

class MEngineObject
{
public:
	int32_t PosX	= 0;
	int32_t PosY	= 0;
	int32_t Width	= 0;
	int32_t Height	= 0;

	MEngineEntityManager::MEngineEntityID EntityID	= INVALID_MENGINE_ENTITY_ID;
	MEngineGraphics::MEngineTextureID TextureID		= INVALID_MENGINE_TEXTURE_ID;

protected:
	MEngineObject() {}
};