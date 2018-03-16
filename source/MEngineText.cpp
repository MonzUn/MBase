#include "Interface/MEngineText.h"
#include "MengineTextInternal.h"
#include "Interface/MEngineColor.h"
#include "Interface/MEngineEntityManager.h"
#include "Interface/MEngineInternalComponents.h"
#include "Interface/MEngineInput.h"
#include "Interface/MEngineUtility.h"
#include "MEngineGraphicsInternal.h"
#include <MUtilityIDBank.h>
#include <MUtilityLog.h>
#include <SDL_FontCache.h>
#include <vector>

#define LOG_CATEGORY_TEXT "MEngineText"

namespace MEngine
{
	std::vector<FC_Font*>*		m_Fonts;
	MUtility::MUtilityIDBank*	m_FontIDBank;
}

using namespace MEngine;
using namespace MEngineText;

// ---------- INTERFACE ----------

MEngineFontID MEngine::CreateFont(const std::string& relativeFontPath, int32_t fontSize, const ColorData& textColor)
{
	MEngineFontID ID = INVALID_MENGINE_FONT_ID;
	FC_Font* font = FC_CreateFont();
	const std::string absolutePath = MEngine::GetExecutablePath() + '/' + relativeFontPath;
	if (!FC_LoadFont(font, MEngineGraphics::GetRenderer(), absolutePath.c_str(), fontSize, FC_MakeColor(textColor.R, textColor.G, textColor.B, textColor.A), TTF_STYLE_NORMAL))
	{
		MLOG_WARNING("Failed to load font at path \"" << absolutePath << '\"', LOG_CATEGORY_TEXT);
		FC_FreeFont(font);
		font = nullptr;
	}

	if (font != nullptr)
	{
		ID = m_FontIDBank->GetID();
		if (ID == m_FontIDBank->PeekNextID() - 1)
			m_Fonts->push_back(font);
		else
			(*m_Fonts)[ID] = font;
	}

	return ID;
}

bool MEngine::DestroyFont(MEngineFontID ID)
{
	if (m_FontIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to destroy font using an inactive font ID; ID = " << ID, LOG_CATEGORY_TEXT);
		return false;
	}

	bool result = m_FontIDBank->ReturnID(ID);
	if (result)
	{
		FC_FreeFont((*m_Fonts)[ID]);
		(*m_Fonts)[ID] = nullptr;
	}
	return result;
}

int32_t MEngine::GetTextWidth(MEngineFontID ID, const char* text)
{
	if (!m_FontIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to get text witdh using an inactive font ID; ID = " << ID, LOG_CATEGORY_TEXT);
		return -1;
	}

	return FC_GetWidth((*m_Fonts)[ID], text);
}

int32_t MEngine::GetTextHeight(MEngineFontID ID, const char* text)
{
	if (!m_FontIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to get text height using an inactive font ID; ID = " << ID, LOG_CATEGORY_TEXT);
		return -1;
	}

	return FC_GetHeight((*m_Fonts)[ID], text);
}

int32_t MEngine::GetLineHeight(MEngineFontID ID)
{
	if (!m_FontIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to get line height using an inactive font ID; ID = " << ID, LOG_CATEGORY_TEXT);
		return -1;
	}

	return FC_GetLineHeight((*m_Fonts)[ID]);
}

bool MEngine::IsFontIDValid(MEngineFontID ID)
{
	return m_FontIDBank->IsIDActive(ID);
}

// ---------- INTERNAL ----------

void MEngineText::Initialize()
{
	m_Fonts			= new std::vector<FC_Font*>();
	m_FontIDBank	= new MUtility::MUtilityIDBank();
}

void MEngineText::Shutdown()
{
	for (int i = 0; i < m_Fonts->size(); ++i)
	{
		FC_FreeFont((*m_Fonts)[i]);
	}
	delete m_Fonts;
	delete m_FontIDBank;
}

FC_Font* MEngineText::GetFont(MEngineFontID ID)
{
	if (!m_FontIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to get font object using an inactive font ID; ID = " << ID, LOG_CATEGORY_TEXT);
		return nullptr;
	}

	return (*m_Fonts)[ID];
}