#pragma once

namespace MEngineText
{
	enum class TextRenderJobType
	{
		PLAIN,
		BOX,

		INVALID,
	};

	struct TextRenderJob
	{
		TextRenderJob(int32_t posX, int32_t posY, const char* text)
			: Type(TextRenderJobType::PLAIN), PosX(posX), PosY(posY)
		{
			CopyText(text);
		}

		TextRenderJob(int32_t posX, int32_t posY, int32_t width, int32_t height, const char* text)
			: Type(TextRenderJobType::BOX), PosX(posX), PosY(posY), Width(width), Height(height)
		{
			CopyText(text);
		}

		TextRenderJob(const TextRenderJob& other)
			: Type(other.Type), PosX(other.PosX), PosY(other. PosY), Width(other.Width), Height(other.Height)
		{
			CopyText(other.Text);
		}

		~TextRenderJob() { delete[] Text; Text = nullptr; }

		const int32_t			PosX		= -1;
		const int32_t			PosY		= -1;
		const int32_t			Width		= -1;
		const int32_t			Height		= -1;
		const int32_t			CaretIndex	= -1;
		const char*				Text		= nullptr;
		const TextRenderJobType Type		= TextRenderJobType::INVALID;

	private:
		void CopyText(const char* str)
		{
			char* tempText = new char[strlen(str) + 1]; // +1 for null terminator
			strcpy(tempText, str);
			Text = tempText;
		}
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
	void Update();

	void Render(); // TODODB: Move to MEngineGraphics
}