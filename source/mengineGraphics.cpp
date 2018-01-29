#include "mengineGraphicsInternal.h"
#include "mengineEntityManagerInternal.h"
#include "sdlLock.h"
#include "interface/mengineObject.h"
#include <MUtilityLog.h>
#include <MUtilityPlatformDefinitions.h>
#include <MUtilityWindowsInclude.h>
#include <SDL.h>
#include <mutex>

using namespace MEngineGraphics;

#define MUTILITY_LOG_CATEGORY_GRAPHICS "MEngineGraphics" // TODODB: Rename this MEngine instead of MUtility

namespace MEngineGraphics
{
	std::vector<MEngineTexture*> Textures;
	std::vector<MEngineTextureID> RecycledIDs;
	std::mutex IdLock;
}

// ---------- INTERFACE ----------

void MEngineGraphics::UnloadTexture(MEngineTextureID textureID)
{
	HandleSurfaceToTextureConversions();

	MEngineTexture*& texture = Textures[textureID];
	if (texture != nullptr)
	{
		delete texture;
		texture = nullptr;
		IdLock.lock();
		RecycledIDs.push_back(textureID);
		IdLock.unlock();
	}
	else
	{
		bool isRecycled = false;
		IdLock.lock();
		for (int i = 0; i < RecycledIDs.size(); ++i)
		{
			if (RecycledIDs[i] == textureID)
			{
				isRecycled = true;
				break;
			}
		}
		IdLock.unlock();

		if (isRecycled)
			MLOG_WARNING("Attempted to unload texture with ID " << textureID << " but the texture with that ID has already been unloaded", MUTILITY_LOG_CATEGORY_GRAPHICS);
		else
			MLOG_WARNING("Attempted to unload texture with ID " << textureID << " but no texture with that ID exists", MUTILITY_LOG_CATEGORY_GRAPHICS);
	}
}

MEngineTextureID MEngineGraphics::CreateSubTextureFromTextureData(const MEngineTextureData& originalTexture, int32_t posX, int32_t posY, int32_t width, int32_t height, bool storeCopyInRAM)
{
	int32_t offsetLimitX = posX + width;
	int32_t offsetLimitY = posY + height;

	if (posX < 0 || posY < 0 || offsetLimitX >= originalTexture.Width || offsetLimitY >= originalTexture.Height)
	{
		MLOG_WARNING("Invalid clip information supplied [" << originalTexture.Width << ',' << originalTexture.Height << ']' << ' (' << posX << ',' << posY << ") (" << (posX + width) << ',' << (posY + height) << ')', MUTILITY_LOG_CATEGORY_GRAPHICS);
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

	MEngineTextureID reservedID = GetNextTextureID();
	SurfaceToTextureQueue.Produce(new SurfaceToTextureJob(convertedSurface, reservedID, storeCopyInRAM));

	SdlApiLock.lock();
	SDL_FreeSurface(surface);
	SdlApiLock.unlock();

	return reservedID;
}

MEngineTextureID MEngineGraphics::CreateTextureFromTextureData(const MEngineTextureData& textureData, bool storeCopyInRAM)
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

	MEngineTextureID reservedID = GetNextTextureID();
	SurfaceToTextureQueue.Produce(new SurfaceToTextureJob(convertedSurface, reservedID, storeCopyInRAM));

	SdlApiLock.lock();
	SDL_FreeSurface(surface);
	SdlApiLock.unlock();

	return reservedID;
}

MEngineTextureID MEngineGraphics::CaptureScreenToTexture(bool storeCopyInRAM)
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

		MEngineTextureID reservedID = GetNextTextureID();
		SurfaceToTextureQueue.Produce(new SurfaceToTextureJob(convertedSurface, reservedID, storeCopyInRAM));

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

void MEngineGraphics::SetRenderIgnore(MEngineTextureID textureID, bool shouldIgnoreTexture)
{
	if (textureID != INVALID_MENGINE_TEXTURE_ID && textureID < static_cast<int64_t>(Textures.size()))
		Textures[textureID]->renderIgnore = shouldIgnoreTexture;
	else
		MLOG_WARNING("Attempted to set RenderIgnore for invalid texture ID; ID = " << textureID, MUTILITY_LOG_CATEGORY_GRAPHICS);
}

const MEngineTextureData MEngineGraphics::GetTextureData(MEngineTextureID textureID)
{
	HandleSurfaceToTextureConversions();

	MEngineTextureData toReturn;
	MEngineTexture* texture = nullptr;
	if (textureID != INVALID_MENGINE_TEXTURE_ID && textureID <static_cast<int64_t>(Textures.size()))
	{
		const MEngineTexture& texture = *Textures[textureID];
		toReturn = MEngineTextureData(texture.surface->w, texture.surface->h, texture.surface->pixels, texture.renderIgnore);
	}
	else
		MLOG_WARNING("Attempted to get Texture from invalid texture ID; ID = " << textureID, MUTILITY_LOG_CATEGORY_GRAPHICS);

	return toReturn;
}

// ---------- INTERNAL ----------

bool MEngineGraphics::Initialize(const char* appName, int32_t windowWidth, int32_t windowHeight)
{
	Window = SDL_CreateWindow(appName, 100, 100, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
	if (Window == nullptr)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_CreateWindow Error: " + std::string(SDL_GetError()), MUTILITY_LOG_CATEGORY_GRAPHICS);
		return false;
	}
	
	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (Renderer == nullptr)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_CreateRenderer Error: " + std::string(SDL_GetError()), MUTILITY_LOG_CATEGORY_GRAPHICS);
		return false;
	}
	return true;
}

MEngineTextureID MEngineGraphics::AddTexture(SDL_Texture* sdlTexture, SDL_Surface* optionalSurfaceCopy, MEngineTextureID reservedTextureID)
{
	MEngineTexture* texture = new MEngineTexture(sdlTexture, optionalSurfaceCopy);

	MEngineTextureID ID = reservedTextureID == INVALID_MENGINE_TEXTURE_ID ? GetNextTextureID() : reservedTextureID;
	ID >= static_cast<int64_t>(Textures.size()) ? Textures.push_back(texture) : Textures[ID] = texture;
	return ID;
}

void MEngineGraphics::HandleSurfaceToTextureConversions()
{
	SurfaceToTextureJob* job;
	while (SurfaceToTextureQueue.Consume(job))
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

MEngineTextureID MEngineGraphics::GetNextTextureID()
{
	static MEngineTextureID nextID = 0;

	MEngineTextureID toReturn;
	IdLock.lock();
	if (RecycledIDs.size() > 0)
	{
		toReturn = RecycledIDs.back();
		RecycledIDs.pop_back();
	}
	else
		toReturn = nextID++;
	IdLock.unlock();

	return toReturn;
}

void MEngineGraphics::Render()
{
	HandleSurfaceToTextureConversions();

	SdlApiLock.lock();
	SDL_RenderClear(Renderer);
	RenderEntities();
	SDL_RenderPresent(Renderer);
	SdlApiLock.unlock();
}

void MEngineGraphics::RenderEntities()
{
	const std::vector<MEngineObject*>& entities = MEngineEntityManager::GetEntities();
	for (int i = 0; i < entities.size(); ++i)
	{
		if (entities[i] != nullptr && entities[i]->TextureID != INVALID_MENGINE_TEXTURE_ID)
		{
			if (!Textures[entities[i]->TextureID]->renderIgnore)
			{
				SDL_Rect destinationRect = SDL_Rect();
				destinationRect.x = entities[i]->PosX;
				destinationRect.y = entities[i]->PosY;
				destinationRect.w = entities[i]->Width;
				destinationRect.h = entities[i]->Height;

				int result = SDL_RenderCopy(Renderer, Textures[entities[i]->TextureID]->texture, nullptr, &destinationRect);
				if (result != 0)
					MLOG_WARNING("Failed to render texture with ID: " << entities[i]->TextureID << '\n' << "SDL error = \"" << SDL_GetError() << "\" \n", MUTILITY_LOG_CATEGORY_GRAPHICS);
			}
		}
	}
}