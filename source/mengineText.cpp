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
	std::vector<TextRenderJob>* TextRenderJobs;
	std::vector<CaretRenderJob>* CaretRenderJobs;
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
	TextRenderJobs->push_back(TextRenderJob(posX, posY, text.c_str()));
}

void MEngineText::DrawTextWithCaret(int32_t posX, int32_t posY, const std::string& text, uint16_t caretIndex)
{
	if (caretIndex >= 0 && caretIndex <= text.length())
	{
		DrawText(posX, posY, text.c_str());

		std::string measureString = text.substr(0, caretIndex);
		uint16_t width = FC_GetWidth(Font, measureString.c_str());
		uint16_t height = FC_GetHeight(Font, measureString.c_str());
		CaretRenderJobs->push_back(CaretRenderJob(posX + width, posY, height));
	}
	else
		MLOG_WARNING("Attempted to draw cursor at position outside of string; string = \"" << text << "\"; cursor position = " << caretIndex, MUtility_LOG_CATEGORY_TEXT);
}

// ---------- INTERNAL ----------

void MEngineText::Initialize()
{
	TextRenderJobs = new std::vector<TextRenderJob>();
	CaretRenderJobs = new std::vector<CaretRenderJob>();
}

void MEngineText::Shutdown()
{
	FreeFont(Font);

	delete TextRenderJobs;
	delete CaretRenderJobs;
}

void MEngineText::Render()
{
	for(int i = 0; i < TextRenderJobs->size(); ++i)
	{
		FC_Draw(Font, MEngineGraphics::GetRenderer(), static_cast<float>((*TextRenderJobs)[i].PosX), static_cast<float>((*TextRenderJobs)[i].PosY), (*TextRenderJobs)[i].Text);
	}
	TextRenderJobs->clear();

	for (int i = 0; i < CaretRenderJobs->size(); ++i)
	{
		SDL_RenderDrawLine(MEngineGraphics::GetRenderer(), (*CaretRenderJobs)[i].PosX, (*CaretRenderJobs)[i].TopPosY, (*CaretRenderJobs)[i].PosX, (*CaretRenderJobs)[i].TopPosY + (*CaretRenderJobs)[i].Height);
	}
	CaretRenderJobs->clear();
}

// ---------- INTERNAL ----------

void FreeFont(FC_Font*& font)
{
	FC_FreeFont(font);
	font = nullptr;
}