#pragma once
#include "mengineComponent.h"
#include "MUtilityTypes.h"
#include <MUtilityMacros.h>

namespace MEngine
{
	class TextureRenderingComponent : public ComponentBase<TextureRenderingComponent>
	{
	public:
		int32_t PosX	= 0;
		int32_t PosY	= 0;
		int32_t Width	= 0;
		int32_t Height	= 0;
		bool RenderIgnore = false;
		MEngineTextureID TextureID = INVALID_MENGINE_TEXTURE_ID;
	};
}