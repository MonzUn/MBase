#pragma once
#include "mengineTypes.h"
#include <cstdint>
#include <string>

#define MENGINE_BYTES_PER_PIXEL 4  // RGBA

namespace MEngineGraphics
{
	struct MEngineTextureData
	{
		MEngineTextureData() {}
		MEngineTextureData(int32_t width, int32_t height, const void* pixels = nullptr, bool renderIgnore = false) : Width(width), Height(height), Pixels(pixels) {}

		const void* Pixels			= nullptr;
		int32_t		Width			= -1;
		int32_t		Height			= -1;
	};

	MEngineTextureID	GetTextureFromPath(const std::string& pathWithExtension);
	void				UnloadTexture(MEngineTextureID textureID);

	MEngineTextureID	CreateSubTextureFromTextureData(const MEngineTextureData& originalTexture, int32_t upperLeftOffsetX, int32_t upperLeftOffsetY, int32_t lowerRightOffsetX, int32_t lowerRightOffsetY, bool storeCopyInRAM = false);
	MEngineTextureID	CreateTextureFromTextureData(const MEngineTextureData& textureData, bool storeCopyInRAM = false);
	MEngineTextureID	CaptureScreenToTexture(bool storeCopyInRAM = false);

	const MEngineTextureData GetTextureData(MEngineTextureID textureID);
}