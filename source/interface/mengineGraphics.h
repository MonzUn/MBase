#pragma once
#include <cstdint>
#include <string>

#define INVALID_MENGINE_TEXTURE_ID -1;

namespace MEngineGraphics
{
	typedef int64_t MEngineTextureID;

	void				UnloadTexture(MEngineTextureID textureID);
	MEngineTextureID	CaptureScreenToTexture();
}