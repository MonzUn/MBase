#pragma once

namespace MEngineText
{
	struct TextRenderJob
	{
		TextRenderJob(int32_t posX, int32_t posY, const char* text) : PosX(posX), PosY(posY)
		{
			char* tempText = new char[strlen(text) + 1]; // +1 for null terminator
			strcpy(tempText, text);
			Text = tempText;
		}

		TextRenderJob(const TextRenderJob& other) : PosX(other.PosX), PosY(other. PosY)
		{
			char* tempText = new char[strlen(other.Text) + 1]; // +1 for null terminator
			strcpy(tempText, other.Text);
			Text = tempText;
		}

		~TextRenderJob() { delete[] Text; Text = nullptr; }

		const int32_t		PosX		= -1;
		const int32_t		PosY		= -1;
		const int32_t		CaretIndex	= -1;
		const char*			Text		= nullptr;
	};

	struct CaretRenderJob
	{
		CaretRenderJob(int32_t PosX, int32_t topPosY, uint32_t height) : PosX(PosX), TopPosY(topPosY), Height(height) {}

		const int32_t PosX;
		const int32_t TopPosY;
		const uint32_t Height;
	};

	void Initialize();
	void Shutdown();

	void Render();
}