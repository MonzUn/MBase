#include "mengineGraphicsInternal.h"
#include "mengineEntityManagerInternal.h"
#include "interface/mengineLog.h"
#include "interface/mengineObject.h"
#include "utilities/platformDefinitions.h"
#include <SDL.h>

#if PLATFORM == PLATFORM_WINDOWS
#include "utilities/windowsInclude.h"
#endif

using namespace MEngineGraphics;

#define MENGINE_LOG_CATEGORY_GRAPHICS "MEngineGraphics"

std::vector<MEngineTexture*> Textures;
std::vector<MEngineTextureID> RecycledIDs;

MEngineTextureID GetNextTextureID();

void MEngineGraphics::UnloadTexture(MEngineTextureID textureID)
{
	MEngineTexture* texture = Textures[textureID];
	if (texture != nullptr)
	{
		SDL_DestroyTexture(texture->texture);
		delete texture;
		texture = nullptr;
		RecycledIDs.push_back(textureID);
	}
	else
	{
		bool isRecycled = false;
		for (int i = 0; i < RecycledIDs.size(); ++i)
		{
			if (RecycledIDs[i] == textureID)
			{
				isRecycled = true;
				break;
			}
		}

		if (isRecycled)
			MLOG_WARNING("Attempted to unload texture with ID " << textureID << " but the texture with that ID has already been unloaded", MENGINE_LOG_CATEGORY_GRAPHICS);
		else
			MLOG_WARNING("Attempted to unload texture with ID " << textureID << " but no texture with that ID exists", MENGINE_LOG_CATEGORY_GRAPHICS);
	}
}

MEngineTextureID MEngineGraphics::CaptureScreenToTexture()
{
#if PLATFORM != PLATFORM_WINDOWS
		static_assert(false, "CaptureScreen is only supported on the windows platform");
#else
		int32_t screenWidth		= GetSystemMetrics(SM_CXSCREEN);
		int32_t screenHeight	= GetSystemMetrics(SM_CYSCREEN);

		HWND	desktopWindow			= GetDesktopWindow();
		HDC		desktopDeviceContext	= GetDC(desktopWindow);
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

		SDL_Surface* surface = SDL_CreateRGBSurface(SDL_SWSURFACE, header.biWidth, header.biHeight, header.biBitCount, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

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

		// Copy bits onto the surface
		if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
		memcpy(surface->pixels, flippedPixels, header.biSizeImage);
		if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

		// Convert surface to display format
		SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_GetWindowPixelFormat(Window), NULL);
		SDL_Texture* texture = SDL_CreateTextureFromSurface(Renderer, convertedSurface);
		
		MEngineTextureID ID = MEngineGraphics::AddTexture(texture);

		// Cleanup
		DeleteDC(desktopDeviceContext);
		DeleteDC(captureDeviceContext);
		delete[] pixels;
		delete[] flippedPixels;
		SDL_FreeSurface(surface);

		return ID;
#endif
}

bool MEngineGraphics::Initialize(const char* appName, int32_t windowWidth, int32_t windowHeight)
{
	Window = SDL_CreateWindow(appName, 100, 100, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
	if (Window == nullptr)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_CreateWindow Error: " + std::string(SDL_GetError()) + "; program will close", MENGINE_LOG_CATEGORY_GRAPHICS);
		return false;
	}

	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (Renderer == nullptr)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_CreateRenderer Error: " + std::string(SDL_GetError()) + "; program will close", MENGINE_LOG_CATEGORY_GRAPHICS);
		return false;
	}

	return true;
}

MEngineTextureID MEngineGraphics::AddTexture(SDL_Texture* sdlTexture)
{
	MEngineTextureID ID = GetNextTextureID();
	MEngineTexture* texture = new MEngineTexture(sdlTexture);
	ID >= static_cast<int64_t>(Textures.size()) ? Textures.push_back(texture) : Textures[ID] = texture;
	return ID;
}

void MEngineGraphics::Render()
{
	SDL_RenderClear(Renderer);
	RenderEntities();
	SDL_RenderPresent(Renderer);
}

void MEngineGraphics::RenderEntities()
{
	const std::vector<MEngineObject*>& entites = MEngineEntityManager::GetEntities();
	for (int i = 0; i < entites.size(); ++i)
	{
		int result = SDL_RenderCopy(Renderer, Textures[entites[i]->TextureID]->texture, nullptr, nullptr);
		if (result != 0)
			MLOG_WARNING("Failed to render texture with ID: " << entites[i]->TextureID << '\n' << "SDL error = \"" << SDL_GetError() << "\" \n" , "MENGINE_LOG_CATEGORY_GRAPHICS");
	}
}

MEngineTextureID GetNextTextureID()
{
	static MEngineTextureID nextID = 0;
	
	MEngineTextureID recycledID = -1;
	if (RecycledIDs.size() > 0)
	{
		recycledID = RecycledIDs.back();
		RecycledIDs.pop_back();
	}
	
	return recycledID >= 0 ? recycledID : nextID++;
}