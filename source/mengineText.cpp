#include "interface/mengineText.h"
#include "mengineTextInternal.h"
#include "interface/mengineColor.h"
#include "interface/mengineInput.h"
#include "interface/mengineUtility.h"
#include "mengineGraphicsInternal.h"
#include <MUtilityLog.h>
#include <SDL_FontCache.h>
#include <vector>

#define LOG_CATEGORY_TEXT "MEngineText"

constexpr int32_t DEFAULT_POINT_SIZE = 20;
constexpr int32_t CARET_HEIGHT_OFFSET_TOP = 3;
constexpr int32_t CARET_HEIGHT_OFFSET_BOTTOM = 2;
constexpr int32_t CARET_END_OF_STRING_PFFSET = 1;
const MEngine::ColorData DEFAULT_TEXT_COLOR = MEngine::ColorData(MEngine::PredefinedColors::BLACK);

void FreeFont(FC_Font*& font);

namespace MEngine
{
	FC_Font* m_Font = nullptr;
	std::vector<MEngineText::TextRenderJob>* m_TextRenderJobs;
	std::vector<MEngineText::CaretRenderJob>* m_CaretRenderJobs;
}

using namespace MEngine;
using namespace MEngineText;

// ---------- INTERFACE ----------

void MEngine::SetFont(const std::string& relativeFontPath)
{
	if (m_Font != nullptr)
		FreeFont(m_Font);

	m_Font = FC_CreateFont();
	const std::string absolutePath = MEngine::GetExecutablePath() + '/' + relativeFontPath;
	if (!FC_LoadFont(m_Font, MEngineGraphics::GetRenderer(), absolutePath.c_str(), DEFAULT_POINT_SIZE, FC_MakeColor(DEFAULT_TEXT_COLOR.R, DEFAULT_TEXT_COLOR.G, DEFAULT_TEXT_COLOR.B, DEFAULT_TEXT_COLOR.A), TTF_STYLE_NORMAL))
	{
		MLOG_WARNING("Failed to load font at path \"" << absolutePath << '\"', LOG_CATEGORY_TEXT);
		FreeFont(m_Font);
	}
}

void MEngine::DrawText(int32_t posX, int32_t posY, const std::string& text)
{
	if(text != "")
		m_TextRenderJobs->push_back(TextRenderJob(posX, posY, text.c_str()));
}
}

void MEngine::DrawTextWithCaret(int32_t posX, int32_t posY, const std::string& text, int32_t caretIndex)
{
	if (caretIndex == -1)
		caretIndex = static_cast<int32_t>(MEngine::GetTextInputCaretIndex());

	if (caretIndex >= 0 && caretIndex <= text.length())
	{
		DrawText(posX, posY, text.c_str());

		std::string measureString = text.substr(0, caretIndex);
		uint16_t width = FC_GetWidth(m_Font, measureString.c_str());
		uint16_t height = FC_GetHeight(m_Font, measureString.c_str());

		if (caretIndex >= text.length())
			posX += CARET_END_OF_STRING_PFFSET;

		m_CaretRenderJobs->push_back(CaretRenderJob(posX + width, posY, height));
	}
	else
		MLOG_WARNING("Attempted to draw cursor at position outside of string; string = \"" << text << "\"; cursor position = " << caretIndex, LOG_CATEGORY_TEXT);
}

uint16_t MEngine::GetTextWidth(const char* text)
{
	return FC_GetWidth(m_Font, text);
}

uint16_t MEngine::GetTextHeight(const char* text)
{
	return FC_GetHeight(m_Font, text);
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
		const TextRenderJob& textJob = (*m_TextRenderJobs)[i];
		FC_Draw(m_Font, MEngineGraphics::GetRenderer(), static_cast<float>(textJob.PosX), static_cast<float>(textJob.PosY), textJob.Text);
	}
	m_TextRenderJobs->clear();

	for (int i = 0; i < m_CaretRenderJobs->size(); ++i)
	{
		const CaretRenderJob& caretJob = (*m_CaretRenderJobs)[i];
		SDL_RenderDrawLine(MEngineGraphics::GetRenderer(), caretJob.PosX, caretJob.TopPosY + CARET_HEIGHT_OFFSET_TOP, caretJob.PosX, caretJob.TopPosY + caretJob.Height - CARET_HEIGHT_OFFSET_BOTTOM);
	}
	m_CaretRenderJobs->clear();
}

// ---------- INTERNAL ----------

void FreeFont(FC_Font*& font)
{
	FC_FreeFont(font);
	font = nullptr;
}