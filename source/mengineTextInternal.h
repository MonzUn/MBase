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

		TextRenderJob(const TextRenderJob& other)
		{
			PosX = other.PosX;
			PosY = other.PosY;

			char* tempText = new char[strlen(other.Text) + 1]; // +1 for null terminator
			strcpy(tempText, other.Text);
			Text = tempText;
		}

		~TextRenderJob() { delete[] Text; Text = nullptr; }

		int32_t		PosX	= -1;
		int32_t		PosY	= -1;
		const char* Text	= nullptr;
	};

	void Shutdown();

	void Render();
}