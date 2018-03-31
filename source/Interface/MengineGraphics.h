#pragma once
#include "mengineTypes.h"
#include <cstdint>
#include <string>

#define MENGINE_BYTES_PER_PIXEL 4  // RGBA

namespace MEngine
{
	struct TextureData
	{
		TextureData() {}
		TextureData(int32_t width, int32_t height, const void* pixels = nullptr, bool renderIgnore = false) : Width(width), Height(height), Pixels(pixels) {}

		const void* Pixels			= nullptr;
		int32_t		Width			= -1;
		int32_t		Height			= -1;
	};

	TextureID	GetTextureFromPath(const std::string& pathWithExtension);
	void		UnloadTexture(TextureID textureID);

	TextureID	CreateSubTextureFromTextureData(const TextureData& originalTexture, int32_t upperLeftOffsetX, int32_t upperLeftOffsetY, int32_t lowerRightOffsetX, int32_t lowerRightOffsetY, bool storeCopyInRAM = false);
	TextureID	CreateTextureFromTextureData(const TextureData& textureData, bool storeCopyInRAM = false);
	TextureID	CaptureScreenToTexture(bool storeCopyInRAM = false);

	const TextureData GetTextureData(TextureID textureID);

	int32_t		GetDisplayWidth(int32_t displayIndex);
	int32_t		GetDisplayHeight(int32_t displayIndex);
	int32_t		GetWindowWidth();
	int32_t		GetWindowHeight();
	int32_t		GetWindowPosX();
	int32_t		GetWindowPosY();
}