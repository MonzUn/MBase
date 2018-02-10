#include "interface/mengineText.h"
#include "mengineTextInternal.h"
#include "interface/mengineUtility.h"
#include "interface/mengineColor.h"
#include "mengineGraphicsInternal.h"
#include <MUtilityLog.h>
#include <SDL_FontCache.h>
#include <vector>

#define MUtility_LOG_CATEGORY_TEXT "MEngineText"

constexpr int32_t						DEFAULT_POINT_SIZE	= 20;
const MEngineColor::MEngineColorData	DEFAULT_TEXT_COLOR	= MEngineColor::MEngineColorData(MEngineColor::PredefinedColors::BLACK);

void FreeFont(FC_Font*& font);

namespace MEngineText
{
	FC_Font* Font = nullptr;
	std::vector<TextRenderJob> RenderJobs;
}

// ---------- INTERFACE ----------

void MEngineText::SetFont(const std::string& relativeFontPath)
{
	if (Font != nullptr)
		FreeFont(Font);

	Font = FC_CreateFont();
	const std::string absolutePath = MEngineUtility::GetExecutablePath() + '/' + relativeFontPath;
	if (!FC_LoadFont(Font, MEngineGraphics::GetRenderer(), absolutePath.c_str(), DEFAULT_POINT_SIZE, FC_MakeColor(DEFAULT_TEXT_COLOR.R, DEFAULT_TEXT_COLOR.G, DEFAULT_TEXT_COLOR.B, DEFAULT_TEXT_COLOR.A), TTF_STYLE_NORMAL))
	{
		MLOG_WARNING("Failed to load font at path \"" << absolutePath << '\"', MUtility_LOG_CATEGORY_TEXT);
		FreeFont(Font);
	}
}

void MEngineText::DrawText(int32_t posX, int32_t posY, const std::string& text)
{
	RenderJobs.push_back(TextRenderJob(posX, posY, text.c_str()));
}

// ---------- INTERNAL ----------

void MEngineText::Shutdown()
{
	FreeFont(Font);
}

void MEngineText::Render()
{
	for(int i = 0; i < RenderJobs.size(); ++i)
	{
		FC_Draw(Font, MEngineGraphics::GetRenderer(), static_cast<float>(RenderJobs[i].PosX), static_cast<float>(RenderJobs[i].PosY), RenderJobs[i].Text);
	}
	RenderJobs.clear();
}

// ---------- INTERNAL ----------

void FreeFont(FC_Font*& font)
{
	FC_FreeFont(font);
	font = nullptr;
}