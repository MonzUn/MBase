#pragma once
#include <cstdint>
#include <string>

#define INVALID_MENGINE_TEXTURE_ID -1

namespace MEngineGraphics
{
	struct MEngineTextureData
	{
		MEngineTextureData() {}
		MEngineTextureData(int32_t width, int32_t height, const void* pixels = nullptr) : Width(width), Height(height), Pixels(pixels) {}

		const void* Pixels = nullptr;
		int32_t Width = -1;
		int32_t Height = -1;
	};

	typedef int64_t MEngineTextureID;

	void				UnloadTexture(MEngineTextureID textureID);
	MEngineTextureID	CreateTextureFromTextureData(const MEngineTextureData& textureData, bool storeCopyInRAM = false);
	MEngineTextureID	CaptureScreenToTexture(bool storeCopyInRAM = false);

	const MEngineTextureData GetTextureData(MEngineTextureID textureID);
}