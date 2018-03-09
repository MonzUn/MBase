#include "mengineGraphicsInternal.h"
#include "interface/mengineComponentManager.h"
#include "interface/mengineEntityManager.h"
#include "interface/mengineInternalComponents.h"
#include "interface/mengineText.h"
#include "interface/mengineUtility.h"
#include "mengineTextInternal.h"
#include "sdlLock.h"
#include <MUtilityIDBank.h>
#include <MUtilityLocklessQueue.h>
#include <MUtilityLog.h>
#include <MUtilityPlatformDefinitions.h>
#include <MUtilityWindowsInclude.h>
#include <SDL.h>
#include <SDL_FontCache.h>
#include <SDL_image.h>
#include <algorithm>
#include <mutex>
#include <unordered_map>

#define LOG_CATEGORY_GRAPHICS "MEngineGraphics"

constexpr int32_t CARET_HEIGHT_OFFSET_TOP		= 3;
constexpr int32_t CARET_HEIGHT_OFFSET_BOTTOM	= 2;
constexpr int32_t CARET_END_OF_STRING_OFFSET	= 1;

namespace MEngineGraphics
{
	bool IsDeeper(const RenderJob& lhs, const RenderJob& rhs)
	{
		return lhs.Depth > rhs.Depth;
	};

	void CreateRenderJobs();
	void ExecuteRenderJobs();

	SDL_Renderer*	m_Renderer	= nullptr;
	SDL_Window*		m_Window	= nullptr;

	int32_t m_WindowWidth	= -1;
	int32_t m_WindowHeight	= -1;

	std::vector<RenderJob>* m_RenderJobs;
	std::vector<MEngineTexture*>* m_Textures;
	MUtility::MUtilityIDBank* m_IDBank; // TODODB: Rename to textureIDBank
	std::unordered_map<std::string, MEngine::TextureID>* m_PathToIDMap;
	std::mutex m_PathToIDLock;
	MUtility::LocklessQueue<SurfaceToTextureJob*>* m_SurfaceToTextureQueue;
}

using namespace MEngine;
using namespace MEngineGraphics;
using namespace MEngineText;

// ---------- INTERFACE ----------

TextureID MEngine::GetTextureFromPath(const std::string& pathWithExtension)
{
	TextureID returnID = INVALID_MENGINE_TEXTURE_ID;
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
		m_IDBank->ReturnID(textureID);
	}
	else
	{
		if (m_IDBank->IsIDRecycled(textureID))
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
		return INVALID_MENGINE_TEXTURE_ID;
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

TextureID MEngine::CaptureScreenToTexture(bool storeCopyInRAM)
{
#if PLATFORM != PLATFORM_WINDOWS
		static_assert(false, "CaptureScreen is only supported on the windows platform");
#else
		int32_t screenWidth		= GetSystemMetrics(SM_CXSCREEN);
		int32_t screenHeight	= GetSystemMetrics(SM_CYSCREEN);

		HDC		desktopDeviceContext	= GetDC(nullptr);
		HDC		captureDeviceContext	= CreateCompatibleDC(desktopDeviceContext);

		// Take screenshot
		HBITMAP bitMapHandle = CreateCompatibleBitmap(desktopDeviceContext, screenWidth, screenHeight);
		HBITMAP oldBitMapHandle = static_cast<HBITMAP>(SelectObject(captureDeviceContext, bitMapHandle));
		BitBlt(captureDeviceContext, 0, 0, screenWidth, screenHeight, desktopDeviceContext, 0, 0, SRCCOPY | CAPTUREBLT);
		bitMapHandle = static_cast<HBITMAP>(SelectObject(captureDeviceContext, oldBitMapHandle));

		// Get image header information
		BITMAPINFO bitMapInfo = { 0 };
		BITMAPINFOHEADER& header = bitMapInfo.bmiHeader;
		header.biSize = sizeof(header);

		// Get the BITMAPINFO from the bitmap
		GetDIBits(desktopDeviceContext, bitMapHandle, 0, 0, NULL, &bitMapInfo, DIB_RGB_COLORS);
		header.biCompression = BI_RGB;

		// Get the actual bitmap buffer
		BYTE* pixels = new BYTE[header.biSizeImage];
		GetDIBits(desktopDeviceContext, bitMapHandle, 0, header.biHeight, static_cast<LPVOID>(pixels), &bitMapInfo, DIB_RGB_COLORS);

		SdlApiLock.lock();
		SDL_Surface* surface = SDL_CreateRGBSurface(SDL_SWSURFACE, header.biWidth, header.biHeight, header.biBitCount, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
		SdlApiLock.unlock();

		// The vertical orientation is reversed; flip the image upside down
		uint32_t bytesPerRow = header.biSizeImage / header.biHeight;
		BYTE* flippedPixels = new BYTE[header.biSizeImage];

		BYTE* flippedPixelsWalker = flippedPixels;
		BYTE* pixelsWalker = pixels + header.biSizeImage - bytesPerRow;
		for (int i = 0; i < header.biHeight; ++i)
		{
			memcpy(flippedPixelsWalker, pixelsWalker, bytesPerRow);
			flippedPixelsWalker += bytesPerRow;
			pixelsWalker -= bytesPerRow;
		}

		// The image is in BGR format but needs to be in RGB. Flip the values of the R and B positions.
		BYTE temp;
		for (unsigned int i = 0; i < header.biSizeImage; i += MENGINE_BYTES_PER_PIXEL)
		{ 
			temp = flippedPixels[i];
			flippedPixels[i] = flippedPixels[i + 2];
			flippedPixels[i + 2] = temp;
		}

		// Copy bits onto the surface
		SdlApiLock.lock();
		if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface); // TODODB: See if we really need these locks in out case
		memcpy(surface->pixels, flippedPixels, header.biSizeImage);
		if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

		// Convert surface to display format
		SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_GetWindowPixelFormat(m_Window), NULL);
		SdlApiLock.unlock();

		TextureID reservedID = GetNextTextureID();
		m_SurfaceToTextureQueue->Produce(new SurfaceToTextureJob(convertedSurface, reservedID, storeCopyInRAM));

		// Cleanup
		DeleteDC(desktopDeviceContext);
		DeleteDC(captureDeviceContext);
		delete[] pixels;
		delete[] flippedPixels;

		SdlApiLock.lock();
		SDL_FreeSurface(surface);
		SdlApiLock.unlock();

		return reservedID;
#endif
}

const TextureData MEngine::GetTextureData(TextureID textureID)
{
	HandleSurfaceToTextureConversions();

	TextureData toReturn;
	MEngineTexture* texture = nullptr;
	if (textureID != INVALID_MENGINE_TEXTURE_ID && textureID <static_cast<int64_t>(m_Textures->size()))
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

// ---------- INTERNAL ----------

bool MEngineGraphics::Initialize(const char* appName, int32_t windowWidth, int32_t windowHeight)
{
	m_Window = SDL_CreateWindow(appName, 100, 100, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
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

	m_RenderJobs			= new std::vector<RenderJob>();
	m_Textures				= new std::vector<MEngineTexture*>();
	m_IDBank				= new MUtility::MUtilityIDBank();
	m_PathToIDMap			= new std::unordered_map<std::string, TextureID>();
	m_SurfaceToTextureQueue = new MUtility::LocklessQueue<SurfaceToTextureJob*>();

	return true;
}

void MEngineGraphics::Shutdown()
{
	delete m_RenderJobs;

	for (int i = 0; i < m_Textures->size(); ++i)
	{
		delete (*m_Textures)[i];
	}
	delete m_Textures;

	delete m_IDBank;
	delete m_PathToIDMap;
	delete m_SurfaceToTextureQueue;
}

TextureID MEngineGraphics::AddTexture(SDL_Texture* sdlTexture, SDL_Surface* optionalSurfaceCopy, TextureID reservedTextureID)
{
	MEngineTexture* texture = new MEngineTexture(sdlTexture, optionalSurfaceCopy);

	TextureID ID = reservedTextureID == INVALID_MENGINE_TEXTURE_ID ? GetNextTextureID() : reservedTextureID;
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
		MEngineGraphics::AddTexture(texture, (job->StoreSurfaceInRAM ? job->Surface : nullptr), job->ReservedID);

		if (!job->StoreSurfaceInRAM)
			SDL_FreeSurface(job->Surface);
		delete job;
	}
}

TextureID MEngineGraphics::GetNextTextureID()
{
	return m_IDBank->GetID();
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
		RenderJob job;
		ComponentMask entityComponentMask = GetComponentMask(entities[i]);
		if ((entityComponentMask & RectangleRenderingComponent::GetComponentMask()) != 0)
		{
			const RectangleRenderingComponent* rectComp = static_cast<const RectangleRenderingComponent*>(GetComponentForEntity(RectangleRenderingComponent::GetComponentMask(), entities[i]));
			if (!rectComp->RenderIgnore && !rectComp->IsFullyTransparent())
			{
				if (!rectComp->BorderColor.IsFullyTransparent())
					job.BorderColor = rectComp->BorderColor;

				if (!rectComp->FillColor.IsFullyTransparent())
					job.FillColor = rectComp->FillColor;

				job.JobMask |= JobTypeMask::RECTANGLE;
			}
		}

		if ((entityComponentMask & TextureRenderingComponent::GetComponentMask()) != 0)
		{
			const TextureRenderingComponent* textureComp = static_cast<const TextureRenderingComponent*>(GetComponentForEntity(TextureRenderingComponent::GetComponentMask(), entities[i]));
			if (!textureComp->RenderIgnore && textureComp->TextureID != INVALID_MENGINE_TEXTURE_ID)
			{
				job.TextureID = textureComp->TextureID;
				job.JobMask |= JobTypeMask::TEXTURE;
			}
		}

		if ((entityComponentMask & TextComponent::GetComponentMask()) != 0)
		{
			const TextComponent* textComp = static_cast<const TextComponent*>(GetComponentForEntity(TextComponent::GetComponentMask(), entities[i]));
			if (textComp->FontID != INVALID_MENGINE_FONT_ID && textComp->Text != nullptr && *textComp->Text != "")
			{
				job.CopyText(textComp->Text->c_str());
				if (IsInputString(textComp->Text))
				{
					job.CaretIndex	= GetTextInputCaretIndex();
					job.TextWidth	= GetTextWidth(textComp->FontID, job.Text);
					job.TextHeight	= GetTextHeight(textComp->FontID, job.Text);
				}
				job.JobMask |= JobTypeMask::TEXT;
			}
		}

		if (job.JobMask != JobTypeMask::INVALID)
		{
			if ((entityComponentMask & PosSizeComponent::GetComponentMask()) != 0)
			{
				const PosSizeComponent* posSizeComp = static_cast<const PosSizeComponent*>(GetComponentForEntity(PosSizeComponent::GetComponentMask(), entities[i]));
				job.DestinationRect = { posSizeComp->PosX, posSizeComp->PosY, posSizeComp->Width, posSizeComp->Height };
				job.Depth = posSizeComp->PosZ;
				m_RenderJobs->push_back(job);
			}
			else
				MLOG_WARNING("Found entity with a renderable component that lacks position data; entityID = " << entities[i], LOG_CATEGORY_GRAPHICS);
		}
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
		const RenderJob& job = (*m_RenderJobs)[i]; // Guaranteed to have position data
		if ((job.JobMask & JobTypeMask::RECTANGLE) != 0)
		{
			if (!job.FillColor.IsFullyTransparent())
			{
				SDL_SetRenderDrawColor(m_Renderer, job.FillColor.R, job.FillColor.G, job.FillColor.B, job.FillColor.A);
				SDL_RenderFillRect(m_Renderer, &job.DestinationRect);
			}

			if (!job.BorderColor.IsFullyTransparent())
			{
				SDL_SetRenderDrawColor(m_Renderer, job.BorderColor.R, job.BorderColor.G, job.BorderColor.B, job.BorderColor.A);
				SDL_RenderDrawRect(m_Renderer, &job.DestinationRect);
			}
		}

		if ((job.JobMask & JobTypeMask::TEXTURE) != 0)
		{
			int result = SDL_RenderCopy(m_Renderer, (*m_Textures)[job.TextureID]->Texture, nullptr, &job.DestinationRect);
			if (result != 0)
				MLOG_ERROR("Failed to render texture with ID: " << job.TextureID << '\n' << "SDL error = \"" << SDL_GetError() << "\" \n", LOG_CATEGORY_GRAPHICS);
		}

		if ((job.JobMask & JobTypeMask::TEXT) != 0)
		{
			switch (job.TextRenderMode)
			{
				case TextRenderMode::PLAIN:
				{
					FC_Draw(GetFont(job.FontID), m_Renderer, static_cast<float>(job.DestinationRect.x), static_cast<float>(job.DestinationRect.y), job.Text);
				} break;

				case TextRenderMode::BOX:
				{
					FC_DrawBox(GetFont(job.FontID), m_Renderer, job.DestinationRect, job.Text);
				} break;

				case TextRenderMode::INVALID:
				default:
					break;
			}

			if (job.CaretIndex != -1)
			{
				SDL_RenderDrawLine(m_Renderer, job.DestinationRect.x, job.DestinationRect.y + CARET_HEIGHT_OFFSET_TOP, job.DestinationRect.x, job.DestinationRect.y + job.DestinationRect.h - CARET_HEIGHT_OFFSET_BOTTOM);
			}
		}
	}
	m_RenderJobs->clear();

	// Restore draw color
	SDL_SetRenderDrawColor(m_Renderer, startingDrawColor[0], startingDrawColor[1], startingDrawColor[2], startingDrawColor[3]);
}