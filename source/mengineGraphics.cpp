#include "mengineGraphicsInternal.h"
#include "interface/mengineComponentManager.h"
#include "interface/mengineInternalComponents.h"
#include "interface/mengineUtility.h"
#include "mengineTextInternal.h"
#include "sdlLock.h"
#include <MUtilityIDBank.h>
#include <MUtilityLocklessQueue.h>
#include <MUtilityLog.h>
#include <MUtilityPlatformDefinitions.h>
#include <MUtilityWindowsInclude.h>
#include <SDL.h>
#include <SDL_image.h>
#include <mutex>
#include <unordered_map>

using namespace MEngine;
using namespace MEngineGraphics;
using MUtility::MUtilityIDBank;

#define LOG_CATEGORY_GRAPHICS "MEngineGraphics"

namespace MEngineGraphics
{
	SDL_Renderer*	Renderer	= nullptr;
	SDL_Window*		Window		= nullptr;

	std::vector<MEngineTexture*>* m_Textures;
	MUtilityIDBank* m_IDBank;
	std::unordered_map<std::string, TextureID>* m_PathToIDMap;
	std::mutex m_PathToIDLock;
	MUtility::LocklessQueue<SurfaceToTextureJob*>* m_SurfaceToTextureQueue;
}

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
			SDL_Texture* texture = IMG_LoadTexture(Renderer, absolutePath.c_str());
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

	SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_GetWindowPixelFormat(Window), NULL);
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

	SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_GetWindowPixelFormat(Window), NULL);
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
		SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_GetWindowPixelFormat(Window), NULL);
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
		toReturn = TextureData(texture.surface->w, texture.surface->h, texture.surface->pixels);
	}
	else
		MLOG_WARNING("Attempted to get Texture from invalid texture ID; ID = " << textureID, LOG_CATEGORY_GRAPHICS);

	return toReturn;
}

// ---------- INTERNAL ----------

bool MEngineGraphics::Initialize(const char* appName, int32_t windowWidth, int32_t windowHeight)
{
	Window = SDL_CreateWindow(appName, 100, 100, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
	if (Window == nullptr)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_CreateWindow Error: " + std::string(SDL_GetError()), LOG_CATEGORY_GRAPHICS);
		return false;
	}
	
	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (Renderer == nullptr)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_CreateRenderer Error: " + std::string(SDL_GetError()), LOG_CATEGORY_GRAPHICS);
		return false;
	}

	m_Textures = new std::vector<MEngineTexture*>();
	m_IDBank = new MUtilityIDBank();
	m_PathToIDMap = new std::unordered_map<std::string, TextureID>();
	m_SurfaceToTextureQueue = new MUtility::LocklessQueue<SurfaceToTextureJob*>();

	return true;
}

void MEngineGraphics::Shutdown()
{
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
		SDL_Texture* texture = SDL_CreateTextureFromSurface(Renderer, job->Surface );
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
	return Renderer;
}

SDL_Window* MEngineGraphics::GetWindow()
{
	return Window;
}

void MEngineGraphics::Render()
{
	HandleSurfaceToTextureConversions();

	SdlApiLock.lock();
	SDL_RenderClear(Renderer);
	RenderEntities();
	MEngineText::Render();
	SDL_RenderPresent(Renderer);
	SdlApiLock.unlock();
}

void MEngineGraphics::RenderEntities()
{
	int32_t componentCount = 0;
	MEngine::TextureRenderingComponent* textureComponents = reinterpret_cast<MEngine::TextureRenderingComponent*>(MEngine::GetComponentBuffer(MEngine::TextureRenderingComponent::GetComponentMask(), componentCount));

	for (int i = 0; i < componentCount; ++i)
	{
		MEngine::TextureRenderingComponent& textureComponent = textureComponents[i];
		if (textureComponent.RenderIgnore || textureComponent.TextureID == INVALID_MENGINE_TEXTURE_ID)
			continue;

		SDL_Rect destinationRect = SDL_Rect();
		destinationRect.x = textureComponent.PosX;
		destinationRect.y = textureComponent.PosY;
		destinationRect.w = textureComponent.Width;
		destinationRect.h = textureComponent.Height;

		int result = SDL_RenderCopy(Renderer, (*m_Textures)[textureComponent.TextureID]->texture, nullptr, &destinationRect);
		if (result != 0)
			MLOG_ERROR("Failed to render texture with ID: " << textureComponent.TextureID << '\n' << "SDL error = \"" << SDL_GetError() << "\" \n", LOG_CATEGORY_GRAPHICS);
	}
}