#include "interface/mengineText.h"
#include "mengineTextInternal.h"
#include "interface/mengineUtility.h"
#include "interface/mengineColor.h"
#include "mengineGraphicsInternal.h"
#include <MUtilityLog.h>
#include <SDL_FontCache.h>
#include <vector>

#define LOG_CATEGORY_TEXT "MEngineText"

constexpr int32_t						DEFAULT_POINT_SIZE	= 20;
const MEngineColor::MEngineColorData	DEFAULT_TEXT_COLOR	= MEngineColor::MEngineColorData(MEngineColor::PredefinedColors::BLACK);

void FreeFont(FC_Font*& font);

namespace MEngineText
{
	FC_Font* m_Font = nullptr;
	std::vector<TextRenderJob>* m_TextRenderJobs;
	std::vector<CaretRenderJob>* m_CaretRenderJobs;
}

// ---------- INTERFACE ----------

void MEngineText::SetFont(const std::string& relativeFontPath)
{
	if (m_Font != nullptr)
		FreeFont(m_Font);

	m_Font = FC_CreateFont();
	const std::string absolutePath = MEngineUtility::GetExecutablePath() + '/' + relativeFontPath;
	if (!FC_LoadFont(m_Font, MEngineGraphics::GetRenderer(), absolutePath.c_str(), DEFAULT_POINT_SIZE, FC_MakeColor(DEFAULT_TEXT_COLOR.R, DEFAULT_TEXT_COLOR.G, DEFAULT_TEXT_COLOR.B, DEFAULT_TEXT_COLOR.A), TTF_STYLE_NORMAL))
	{
		MLOG_WARNING("Failed to load font at path \"" << absolutePath << '\"', LOG_CATEGORY_TEXT);
		FreeFont(m_Font);
	}
}

void MEngineText::DrawText(int32_t posX, int32_t posY, const std::string& text)
{
	m_TextRenderJobs->push_back(TextRenderJob(posX, posY, text.c_str()));
}

void MEngineText::DrawTextWithCaret(int32_t posX, int32_t posY, const std::string& text, uint16_t caretIndex)
{
	if (caretIndex >= 0 && caretIndex <= text.length())
	{
		DrawText(posX, posY, text.c_str());

		std::string measureString = text.substr(0, caretIndex);
		uint16_t width = FC_GetWidth(m_Font, measureString.c_str());
		uint16_t height = FC_GetHeight(m_Font, measureString.c_str());
		m_CaretRenderJobs->push_back(CaretRenderJob(posX + width, posY, height));
	}
	else
		MLOG_WARNING("Attempted to draw cursor at position outside of string; string = \"" << text << "\"; cursor position = " << caretIndex, LOG_CATEGORY_TEXT);
}

// ---------- INTERNAL ----------

void MEngineText::Initialize()
{
	m_TextRenderJobs = new std::vector<TextRenderJob>();
	m_CaretRenderJobs = new std::vector<CaretRenderJob>();
}

void MEngineText::Shutdown()
{
	FreeFont(m_Font);

	delete m_TextRenderJobs;
	delete m_CaretRenderJobs;
}

void MEngineText::Render()
{
	for(int i = 0; i < m_TextRenderJobs->size(); ++i)
	{
		FC_Draw(m_Font, MEngineGraphics::GetRenderer(), static_cast<float>((*m_TextRenderJobs)[i].PosX), static_cast<float>((*m_TextRenderJobs)[i].PosY), (*m_TextRenderJobs)[i].Text);
	}
	m_TextRenderJobs->clear();

	for (int i = 0; i < m_CaretRenderJobs->size(); ++i)
	{
		SDL_RenderDrawLine(MEngineGraphics::GetRenderer(), (*m_CaretRenderJobs)[i].PosX, (*m_CaretRenderJobs)[i].TopPosY, (*m_CaretRenderJobs)[i].PosX, (*m_CaretRenderJobs)[i].TopPosY + (*m_CaretRenderJobs)[i].Height);
	}
	m_CaretRenderJobs->clear();
}

// ---------- INTERNAL ----------

void FreeFont(FC_Font*& font)
{
	FC_FreeFont(font);
	font = nullptr;
}