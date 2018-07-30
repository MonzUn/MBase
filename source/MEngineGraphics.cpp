#include "MEngineGraphicsInternal.h"
#include "Interface/MengineConfig.h"
#include "Interface/MEngineComponentManager.h"
#include "Interface/MEngineEntityManager.h"
#include "Interface/MEngineInternalComponents.h"
#include "Interface/MEngineText.h"
#include "Interface/MEngineUtility.h"
#include "MEngineTextInternal.h"
#include "sdlLock.h"
#include <MUtilityIDBank.h>
#include <MUtilityLocklessQueue.h>
#include <MUtilityLog.h>
#include <MUtilityMacros.h>
#include <MUtilityPlatformDefinitions.h>
#include <MUtilityWindowsInclude.h>
#include <SDL.h>
#include <SDL_FontCache.h>
#include <SDL_image.h>
#include <algorithm>
#include <mutex>
#include <unordered_map>

#define LOG_CATEGORY_GRAPHICS "MEngineGraphics"

CREATE_NAMESPACED_BITFLAG_OPERATOR_DEFINITIONS(MEngineGraphics, JobTypeMask);

constexpr int32_t CARET_HEIGHT_OFFSET_TOP		= 3;
constexpr int32_t CARET_HEIGHT_OFFSET_BOTTOM	= 5 + CARET_HEIGHT_OFFSET_TOP;
constexpr int32_t CARET_END_OF_STRING_OFFSET	= 2;

using namespace MEngine;
using namespace MEngineGraphics;
using namespace MEngineText;
using namespace PredefinedColors;

namespace MEngineGraphics
{
	bool IsDeeper(const RenderJob* lhs, const RenderJob* rhs)
	{
		return lhs->Depth > rhs->Depth;
	};

	void CreateRenderJobs();
	void ExecuteRenderJobs();

	SDL_Renderer*	m_Renderer	= nullptr;
	SDL_Window*		m_Window	= nullptr;

	int32_t m_DisplayCount					= 0;
	std::vector<SDL_Rect>* m_DispayBounds	= nullptr;

	int32_t m_WindowWidth	= -1;
	int32_t m_WindowHeight	= -1;

	std::vector<RenderJob*>*		m_RenderJobs; // TODODB: Use a frame allocator so that we don't lose performance on all the calls to "new"
	std::vector<MEngineTexture*>*	m_Textures;
	MUtility::MUtilityIDBank<TextureID>*		m_TextureIDBank;
	std::unordered_map<std::string, TextureID>* m_PathToIDMap;
	std::mutex m_PathToIDLock;
	MUtility::LocklessQueue<SurfaceToTextureJob*>* m_SurfaceToTextureQueue;
}

// ---------- INTERFACE ----------

TextureID MEngine::GetTextureFromPath(const std::string& pathWithExtension)
{
	TextureID returnID;
	if (pathWithExtension != "")
	{
		m_PathToIDLock.lock();
		auto iterator = m_PathToIDMap->find(pathWithExtension);
		if (iterator != m_PathToIDMap->end())
		{
			returnID = iterator->second;
		}
		else
		{
			std::string absolutePath = MEngine::GetExecutablePath() + "/" + pathWithExtension;
			SDL_Texture* texture = IMG_LoadTexture(m_Renderer, absolutePath.c_str());
			if (texture != nullptr)
			{
				returnID = AddTexture(texture);
				m_PathToIDMap->insert(std::make_pair(pathWithExtension, returnID));
			}
			else
				MLOG_ERROR("Failed to load texture at path \"" << pathWithExtension << "\"; SDL error = \"" << SDL_GetError() << "\"", LOG_CATEGORY_GRAPHICS);
		}
		m_PathToIDLock.unlock();
	}

	return returnID;
}

void MEngine::UnloadTexture(TextureID textureID)
{
	HandleSurfaceToTextureConversions();

	MEngineTexture*& texture = (*m_Textures)[textureID];
	if (texture != nullptr)
	{
		delete texture;
		texture = nullptr;
		m_TextureIDBank->ReturnID(textureID);
	}
	else
	{
		if (m_TextureIDBank->IsIDRecycled(textureID))
			MLOG_WARNING("Attempted to unload texture with ID " << textureID << " but the texture with that ID has already been unloaded", LOG_CATEGORY_GRAPHICS);
		else
			MLOG_WARNING("Attempted to unload texture with ID " << textureID << " but no texture with that ID exists", LOG_CATEGORY_GRAPHICS);
	}
}

TextureID MEngine::CreateSubTextureFromTextureData(const TextureData& originalTexture, int32_t posX, int32_t posY, int32_t width, int32_t height, bool storeCopyInRAM)
{
	int32_t offsetLimitX = posX + width;
	int32_t offsetLimitY = posY + height;

	if (posX < 0 || posY < 0 || offsetLimitX >= originalTexture.Width || offsetLimitY >= originalTexture.Height)
	{
		MLOG_WARNING("Invalid clip information supplied [" << originalTexture.Width << ',' << originalTexture.Height << ']' << ' (' << posX << ',' << posY << ") (" << (posX + width) << ',' << (posY + height) << ')', LOG_CATEGORY_GRAPHICS);
		return TextureID::Invalid();
	}

	SdlApiLock.lock();
	SDL_Surface* surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

	BYTE* destinationWalker = reinterpret_cast<BYTE*>(surface->pixels);
	const BYTE* sourceWalker = (reinterpret_cast<const BYTE*>(originalTexture.Pixels) + (((originalTexture.Width * MENGINE_BYTES_PER_PIXEL) * posY) + (posX * MENGINE_BYTES_PER_PIXEL)));
	for (int i = posY; i < offsetLimitY; ++i)
	{
		memcpy(destinationWalker, sourceWalker, width * MENGINE_BYTES_PER_PIXEL);
		destinationWalker += width * MENGINE_BYTES_PER_PIXEL;
		sourceWalker += originalTexture.Width * MENGINE_BYTES_PER_PIXEL;
	}
	if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

	SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_GetWindowPixelFormat(m_Window), NULL);
	SdlApiLock.unlock();

	// The image is in BGR format but needs to be in RGB. Flip the values of the R and B positions.
	BYTE* pixelBytes = static_cast<BYTE*>(convertedSurface->pixels);
	int32_t byteCount = convertedSurface->w * convertedSurface->h * MENGINE_BYTES_PER_PIXEL;
	BYTE swap;
	for (int i = 0; i < byteCount; i += MENGINE_BYTES_PER_PIXEL)
	{
		swap = pixelBytes[i];
		pixelBytes[i] = pixelBytes[i + 2];
		pixelBytes[i + 2] = swap;
	}

	TextureID reservedID = GetNextTextureID();
	m_SurfaceToTextureQueue->Produce(new SurfaceToTextureJob(convertedSurface, reservedID, storeCopyInRAM));

	SdlApiLock.lock();
	SDL_FreeSurface(surface);
	SdlApiLock.unlock();

	return reservedID;
}

TextureID MEngine::CreateTextureFromTextureData(const TextureData& textureData, bool storeCopyInRAM)
{
	SdlApiLock.lock();
	SDL_Surface* surface = SDL_CreateRGBSurface(SDL_SWSURFACE, textureData.Width, textureData.Height, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
	memcpy(surface->pixels, textureData.Pixels, textureData.Width * textureData.Height * MENGINE_BYTES_PER_PIXEL);
	if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

	SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_GetWindowPixelFormat(m_Window), NULL);
	SdlApiLock.unlock();

	// The image is in BGR format but needs to be in RGB. Flip the values of the R and B positions.
	BYTE* pixelBytes = static_cast<BYTE*>(convertedSurface->pixels);
	int32_t byteCount = convertedSurface->w * convertedSurface->h * MENGINE_BYTES_PER_PIXEL;
	BYTE swap;
	for (int i = 0; i < byteCount; i += MENGINE_BYTES_PER_PIXEL)
	{
		swap = pixelBytes[i];
		pixelBytes[i] = pixelBytes[i + 2];
		pixelBytes[i + 2] = swap;
	}

	TextureID reservedID = GetNextTextureID();
	m_SurfaceToTextureQueue->Produce(new SurfaceToTextureJob(convertedSurface, reservedID, storeCopyInRAM));

	SdlApiLock.lock();
	SDL_FreeSurface(surface);
	SdlApiLock.unlock();

	return reservedID;
}

const TextureData MEngine::GetTextureData(TextureID textureID)
{
	HandleSurfaceToTextureConversions();

	TextureData toReturn;
	MEngineTexture* texture = nullptr;
	if (textureID.IsValid() && textureID <static_cast<int64_t>(m_Textures->size()))
	{
		const MEngineTexture& texture = *(*m_Textures)[textureID];
		toReturn = TextureData(texture.Surface->w, texture.Surface->h, texture.Surface->pixels);
	}
	else
		MLOG_WARNING("Attempted to get Texture from invalid texture ID; ID = " << textureID, LOG_CATEGORY_GRAPHICS);

	return toReturn;
}

int32_t MEngine::GetWindowWidth()
{
	return m_WindowWidth;
}

int32_t MEngine::GetWindowHeight()
{
	return m_WindowHeight;
}

int32_t MEngine::GetDisplayWidth(int32_t displayIndex)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (displayIndex >= m_DisplayCount)
	{
		MLOG_WARNING("Attempted to get width of display with index " << displayIndex << " but no display with that index exists", LOG_CATEGORY_GRAPHICS);
		return -1;
	}
#endif

	int32_t result = -1;
	SDL_DisplayMode displayMode;
	if (SDL_GetDesktopDisplayMode(displayIndex, &displayMode) == 0)
	{
		result = displayMode.w;
	}
	else
		MLOG_WARNING("Failed to get width of display with index " << displayIndex, LOG_CATEGORY_GRAPHICS);

	return result;
}

int32_t MEngine::GetDisplayHeight(int32_t displayIndex)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (displayIndex >= m_DisplayCount)
	{
		MLOG_WARNING("Attempted to get hegiht of display with index " << displayIndex << " but no display with that index exists", LOG_CATEGORY_GRAPHICS);
		return -1;
	}
#endif

	int32_t result = -1;
	SDL_DisplayMode displayMode;
	if (SDL_GetDesktopDisplayMode(displayIndex, &displayMode) == 0)
	{
		result = displayMode.h;
	}
	else
		MLOG_WARNING("Failed to get height of display with index " << displayIndex, LOG_CATEGORY_GRAPHICS);

	return result;
}

int32_t MEngine::GetWindowPosX()
{
	int32_t windowPosX;
	SDL_GetWindowPosition(GetWindow(), &windowPosX, nullptr);
	return windowPosX;
}

int32_t MEngine::GetWindowPosY()
{
	int32_t windowPosY;
	SDL_GetWindowPosition(GetWindow(), nullptr, &windowPosY);
	return windowPosY;
}

bool MEngine::IsTextureIDValid(TextureID ID)
{
	return m_TextureIDBank->IsIDActive(ID);
}

// ---------- INTERNAL ----------

bool MEngineGraphics::Initialize(const char* appName, int32_t windowPosX, int32_t windowPosY, int32_t windowWidth, int32_t windowHeight)
{
	m_RenderJobs			= new std::vector<RenderJob*>();
	m_Textures				= new std::vector<MEngineTexture*>();
	m_TextureIDBank			= new MUtility::MUtilityIDBank<TextureID>();
	m_PathToIDMap			= new std::unordered_map<std::string, TextureID>();
	m_SurfaceToTextureQueue = new MUtility::LocklessQueue<SurfaceToTextureJob*>();
	m_DispayBounds			= new std::vector<SDL_Rect>();

	m_DisplayCount = SDL_GetNumVideoDisplays();
	for (int i = 0; i < m_DisplayCount; ++i)
	{
		m_DispayBounds->push_back(SDL_Rect());
		SDL_GetDisplayBounds(i, &m_DispayBounds->back());
	}

	const MEngine::InitFlags initFlags = GetInitFlags();
	int32_t initialWindowPosX, initialWindowPosY;
	if ((initFlags & InitFlags::StartWindowCentered) != 0)
	{
		initialWindowPosX = GetDisplayWidth(0) / 2 - windowWidth / 2;
		initialWindowPosY = GetDisplayHeight(0) / 2 - windowHeight / 2;
	}
	
	if ((initFlags & InitFlags::RememberWindowPosition) != 0)
	{
		initialWindowPosX = static_cast<int32_t>(Config::GetInt("WindowPosX", initialWindowPosX));
		initialWindowPosY = static_cast<int32_t>(Config::GetInt("WindowPosY", initialWindowPosY));
	}

	m_Window = SDL_CreateWindow(appName, initialWindowPosX, initialWindowPosY, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
	if (m_Window == nullptr)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_CreateWindow Error: " + std::string(SDL_GetError()), LOG_CATEGORY_GRAPHICS);
		return false;
	}
	SDL_GetWindowSize(m_Window, &m_WindowWidth, &m_WindowHeight);
	
	m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (m_Renderer == nullptr)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_CreateRenderer Error: " + std::string(SDL_GetError()), LOG_CATEGORY_GRAPHICS);
		return false;
	}

	return true;
}

void MEngineGraphics::Shutdown()
{
	if ((GetInitFlags() & InitFlags::RememberWindowPosition) != 0)
	{
		Config::SetInt("WindowPosX", GetWindowPosX());
		Config::SetInt("WindowPosY", GetWindowPosY());
	}

	delete m_RenderJobs;

	for (int i = 0; i < m_Textures->size(); ++i)
	{
		delete (*m_Textures)[i];
	}
	delete m_Textures;

	delete m_TextureIDBank;
	delete m_PathToIDMap;
	delete m_SurfaceToTextureQueue;
	delete m_DispayBounds;
}

TextureID MEngineGraphics::AddTexture(SDL_Texture* sdlTexture, SDL_Surface* optionalSurfaceCopy, TextureID reservedTextureID)
{
	MEngineTexture* texture = new MEngineTexture(sdlTexture, optionalSurfaceCopy);

	TextureID ID = reservedTextureID.IsValid() ? reservedTextureID : GetNextTextureID();
	ID >= static_cast<int64_t>(m_Textures->size()) ? m_Textures->push_back(texture) : (*m_Textures)[ID] = texture;
	return ID;
}

void MEngineGraphics::HandleSurfaceToTextureConversions()
{
	SurfaceToTextureJob* job;
	while (m_SurfaceToTextureQueue->Consume(job))
	{
		SdlApiLock.lock();
		SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, job->Surface );
		SdlApiLock.unlock();
		if (texture != nullptr)
		{
			MEngineGraphics::AddTexture(texture, (job->StoreSurfaceInRAM ? job->Surface : nullptr), job->ReservedID);
		}
		else
		{
			MLOG_ERROR("Failed to create texture from surface", LOG_CATEGORY_GRAPHICS);
			SDL_FreeSurface(job->Surface);
			continue;
		}

		if (!job->StoreSurfaceInRAM)
			SDL_FreeSurface(job->Surface);
		delete job;
	}
}

TextureID MEngineGraphics::GetNextTextureID() // TODODB: Why is this a function?
{
	return m_TextureIDBank->GetID();
}

SDL_Renderer* MEngineGraphics::GetRenderer()
{
	return m_Renderer;
}

SDL_Window* MEngineGraphics::GetWindow()
{
	return m_Window;
}

void MEngineGraphics::Render()
{
	HandleSurfaceToTextureConversions();

	SdlApiLock.lock();
	SDL_RenderClear(m_Renderer);
	CreateRenderJobs();
	ExecuteRenderJobs();
	SDL_RenderPresent(m_Renderer);
	SdlApiLock.unlock();
}

// ---------- LOCAL ----------

void MEngineGraphics::CreateRenderJobs()
{
	std::vector<EntityID> entities;
	ComponentMask compareMask = PosSizeComponent::GetComponentMask() | RectangleRenderingComponent::GetComponentMask() | TextureRenderingComponent::GetComponentMask();
	GetEntitiesMatchingMask(compareMask, entities, MaskMatchMode::Any);

	for (int i = 0; i < entities.size(); ++i)
	{
		RenderJob* job = new RenderJob();
		ComponentMask entityComponentMask = GetComponentMask(entities[i]);
		const PosSizeComponent* posSizeComp = static_cast<const PosSizeComponent*>(GetComponent(entities[i], PosSizeComponent::GetComponentMask()));
		if ((entityComponentMask & PosSizeComponent::GetComponentMask()) != 0)
		{
			job->DestinationRect = { posSizeComp->PosX, posSizeComp->PosY, posSizeComp->Width, posSizeComp->Height };
			job->Depth = posSizeComp->PosZ;
		}
		else
		{
			MLOG_WARNING("Found entity with a renderable component that lacks position data; entityID = " << entities[i], LOG_CATEGORY_GRAPHICS);
			delete job;
			continue;
		}

		if ((entityComponentMask & RectangleRenderingComponent::GetComponentMask()) != 0)
		{
			const RectangleRenderingComponent* rectComp = static_cast<const RectangleRenderingComponent*>(GetComponent(entities[i], RectangleRenderingComponent::GetComponentMask()));
			if (!rectComp->RenderIgnore && !rectComp->IsFullyTransparent())
			{
				if (!rectComp->BorderColor.IsFullyTransparent())
					job->BorderColor = rectComp->BorderColor;

				if (!rectComp->FillColor.IsFullyTransparent())
					job->FillColor = rectComp->FillColor;

				job->JobMask |= JobTypeMask::RECTANGLE;
			}
		}

		if ((entityComponentMask & TextureRenderingComponent::GetComponentMask()) != 0)
		{
			const TextureRenderingComponent* textureComp = static_cast<const TextureRenderingComponent*>(GetComponent(entities[i], TextureRenderingComponent::GetComponentMask()));
			if (!textureComp->RenderIgnore && textureComp->TextureID.IsValid())
			{
				job->TextureID = textureComp->TextureID;
				job->JobMask |= JobTypeMask::TEXTURE;
			}
		}

		if ((entityComponentMask & TextComponent::GetComponentMask()) != 0)
		{
			const TextComponent* textComp = static_cast<const TextComponent*>(GetComponent(entities[i], TextComponent::GetComponentMask()));
			if (!textComp->RenderIgnore && textComp->FontID.IsValid() && textComp->Text != nullptr)
			{
				if (*textComp->Text != "")
				{
					job->FontID = textComp->FontID;
					job->CopyText(textComp->Text->c_str());
					job->TextRenderMode = ((posSizeComp->Width > 0 && posSizeComp->Height > 0) ? TextRenderMode::BOX : TextRenderMode::PLAIN);
					job->TextRect = job->DestinationRect;

					int32_t textHeight = GetTextHeight(textComp->FontID, job->Text);

					// Horizontal alignment
					switch (textComp->Alignment)
					{
						case TextAlignment::TopLeft:
						case TextAlignment::CenterLeft:
						case TextAlignment::BottomLeft:
						{
							job->HorizontalTextAlignment = FC_ALIGN_LEFT;
						} break;
	
						case TextAlignment::TopCentered:
						case TextAlignment::CenterCentered:
						case TextAlignment::BottomCentered:
						{
							job->HorizontalTextAlignment = FC_ALIGN_CENTER;
						} break;
	
						case TextAlignment::TopRight:
						case TextAlignment::CenterRight:
						case TextAlignment::BottomRight:
						{
							job->HorizontalTextAlignment = FC_ALIGN_RIGHT;
						} break;
	
						default:
							break;
					}

					// Vertical alignment
					switch (textComp->Alignment)
					{
						case TextAlignment::CenterLeft:
						case TextAlignment::CenterCentered:
						case TextAlignment::CenterRight:
						{
							job->TextRect.y += (job->DestinationRect.h / 2) - (textHeight / 2);
						} break;

						case TextAlignment::BottomLeft:
						case TextAlignment::BottomCentered:
						case TextAlignment::BottomRight:
						{
							job->TextRect.y += job->DestinationRect.h - textHeight;
						} break;

						case TextAlignment::TopLeft:
						case TextAlignment::TopCentered:
						case TextAlignment::TopRight:
						default:
							break;
					}

					job->JobMask |= JobTypeMask::TEXT;
				}

				// Scroll
				if (textComp->ScrolledLinesCount > 0)
				{
					uint32_t scrollHeight = GetLineHeight(textComp->FontID) * textComp->ScrolledLinesCount;
					job->TextRect.y -= scrollHeight;
					job->TextRect.h += scrollHeight;
				}

				// Caret
				if (IsInputString(textComp->Text))
				{
					if ((job->JobMask & JobTypeMask::TEXT) == 0)
						job->FontID = textComp->FontID;

					// Add caret drawing data
					const uint64_t caretIndex = GetTextInputCaretIndex();

					// TODODB: Put char* substr logic into MUtility
					char* substr = static_cast<char*>(malloc(caretIndex + 1));
					memcpy(substr, textComp->Text->c_str(), caretIndex);
					substr[caretIndex] = '\0';

					job->CaretOffsetX = GetTextWidth(textComp->FontID, substr);
					free(substr);

					if (caretIndex >= strlen(textComp->Text->c_str()))
						job->CaretOffsetX += CARET_END_OF_STRING_OFFSET;

					job->JobMask |= JobTypeMask::CARET;
				}
			}
		}

		if (job->JobMask != JobTypeMask::INVALID)
		{
			m_RenderJobs->push_back(job);
		}
		else
			delete job;
	}
	std::sort(m_RenderJobs->begin(), m_RenderJobs->end(), IsDeeper);
}

void MEngineGraphics::ExecuteRenderJobs()
{
	// Store the draw color used before
	uint8_t startingDrawColor[4];
	SDL_GetRenderDrawColor(m_Renderer, &startingDrawColor[0], &startingDrawColor[1], &startingDrawColor[2], &startingDrawColor[3]);

	for (int i = 0; i < m_RenderJobs->size(); ++i)
	{
		const RenderJob* job = (*m_RenderJobs)[i]; // Guaranteed to have position data
		if ((job->JobMask & JobTypeMask::RECTANGLE) != 0)
		{
			if (!job->FillColor.IsFullyTransparent())
			{
				SDL_SetRenderDrawColor(m_Renderer, job->FillColor.R, job->FillColor.G, job->FillColor.B, job->FillColor.A);
				SDL_RenderFillRect(m_Renderer, &job->DestinationRect);
			}

			if (!job->BorderColor.IsFullyTransparent())
			{
				SDL_SetRenderDrawColor(m_Renderer, job->BorderColor.R, job->BorderColor.G, job->BorderColor.B, job->BorderColor.A);
				SDL_RenderDrawRect(m_Renderer, &job->DestinationRect);
			}
		}

		if ((job->JobMask & JobTypeMask::TEXTURE) != 0)
		{
			int result = SDL_RenderCopy(m_Renderer, (*m_Textures)[job->TextureID]->Texture, nullptr, &job->DestinationRect);
			if (result != 0)
				MLOG_ERROR("Failed to render texture with ID: " << job->TextureID << '\n' << "SDL error Code = " << result << "; SDL error description = \"" << SDL_GetError() << "\" \n", LOG_CATEGORY_GRAPHICS);
		}

		if ((job->JobMask & JobTypeMask::TEXT) != 0)
		{
			switch (job->TextRenderMode)
			{
				case TextRenderMode::PLAIN:
				{
					FC_DrawAlign(GetFont(job->FontID), m_Renderer, static_cast<float>(job->TextRect.x), static_cast<float>(job->TextRect.y), job->HorizontalTextAlignment, job->Text);
				} break;

				case TextRenderMode::BOX:
				{
					FC_DrawBoxAlign(GetFont(job->FontID), m_Renderer, job->TextRect, job->HorizontalTextAlignment, job->Text);
				} break;

				case TextRenderMode::INVALID:
				default:
					break;
			}
		}

		if ((job->JobMask & JobTypeMask::CARET) != 0)
		{
			SDL_SetRenderDrawColor(m_Renderer, Colors[BLACK].R, Colors[BLACK].G, Colors[BLACK].B, Colors[BLACK].A); // TODODB: Make this a settable color
			SDL_RenderDrawLine(m_Renderer, job->DestinationRect.x + job->CaretOffsetX, job->DestinationRect.y + CARET_HEIGHT_OFFSET_TOP, job->DestinationRect.x + job->CaretOffsetX, job->DestinationRect.y + GetLineHeight(job->FontID) - CARET_HEIGHT_OFFSET_BOTTOM);
		}
	}

	for (int i = 0; i < m_RenderJobs->size(); ++i)
	{
		delete (*m_RenderJobs)[i];
	}
	m_RenderJobs->clear();

	// Restore draw color
	SDL_SetRenderDrawColor(m_Renderer, startingDrawColor[0], startingDrawColor[1], startingDrawColor[2], startingDrawColor[3]);
}