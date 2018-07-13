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
#include <limits>
#include <vector>

// TODODB: Optimize so that only the substring that is displayed is handled (GetHeight() does not scale well)

#define LOG_CATEGORY_TEXT "MEngineText"

constexpr uint32_t FontCacheBufferSize = std::numeric_limits<uint32_t>::max();

namespace MEngine
{
	std::vector<FC_Font*>*				m_Fonts;
	MUtility::MUtilityIDBank<FontID>*	m_FontIDBank;
}

using namespace MEngine;
using namespace MEngineText;

// ---------- INTERFACE ----------

FontID MEngine::CreateFont_(const std::string& relativeFontPath, int32_t fontSize, const ColorData& textColor)
{
	FontID ID;
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
		if (m_FontIDBank->IsIDLast(ID))
			m_Fonts->push_back(font);
		else
			(*m_Fonts)[ID] = font;
	}

	return ID;
}

bool MEngine::DestroyFont(FontID ID)
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

int32_t MEngine::GetTextWidth(FontID ID, const char* text)
{
	if (!m_FontIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to get text witdh using an inactive font ID; ID = " << ID, LOG_CATEGORY_TEXT);
		return -1;
	}

	return FC_GetWidth((*m_Fonts)[ID], text);
}

int32_t MEngine::GetTextHeight(FontID ID, const char* text)
{
	if (!m_FontIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to get text height using an inactive font ID; ID = " << ID, LOG_CATEGORY_TEXT);
		return -1;
	}

	return FC_GetHeight((*m_Fonts)[ID], text);
}

int32_t MEngine::GetLineHeight(FontID ID)
{
	if (!m_FontIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to get line height using an inactive font ID; ID = " << ID, LOG_CATEGORY_TEXT);
		return -1;
	}

	return FC_GetLineHeight((*m_Fonts)[ID]);
}

bool MEngine::IsFontIDValid(FontID ID)
{
	return m_FontIDBank->IsIDActive(ID);
}

bool MEngine::IsCharASCII(char character)
{
	return character >= 32 && character <= 126;
}

bool MEngine::IsStringASCII(const char* string)
{
	bool isASCIIOnly = true;
	int32_t stringLength = static_cast<int32_t>(strlen(string));
	for (int i = 0; i < stringLength; ++i)
	{
		if (string[i] < 32 || string[i] > 126)
		{
			isASCIIOnly = false;
			break;
		}
	}
	return isASCIIOnly;
}

// ---------- INTERNAL ----------

void MEngineText::Initialize()
{
	m_Fonts			= new std::vector<FC_Font*>();
	m_FontIDBank	= new MUtility::MUtilityIDBank<FontID>();

	FC_SetBufferSize(FontCacheBufferSize);
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

FC_Font* MEngineText::GetFont(FontID ID)
{
	if (!m_FontIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to get font object using an inactive font ID; ID = " << ID, LOG_CATEGORY_TEXT);
		return nullptr;
	}

	return (*m_Fonts)[ID];
}