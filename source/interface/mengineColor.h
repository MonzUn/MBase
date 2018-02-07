#pragma once
#include <stdint.h>

namespace MEngineColor
{
	namespace PredefinedColors
	{
		enum PredefinedColorEnum : int32_t
		{
			BLACK,
			WHITE,

			COUNT
		};

		constexpr uint8_t PredefinedColors[PredefinedColorEnum::COUNT][4] =
		{
			{ 0,0,0,255 }, // black
			{ 255,255,255,255 } // white
		};
	}

	struct MEngineColorData
	{
		MEngineColorData(PredefinedColors::PredefinedColorEnum color) :
			R(PredefinedColors::PredefinedColors[color][0]), G(PredefinedColors::PredefinedColors[color][1]),
			B(PredefinedColors::PredefinedColors[color][2]), A(PredefinedColors::PredefinedColors[color][3]) {}
		MEngineColorData(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) :
			R(red), G(green), B(blue), A(alpha) {}

		uint8_t R, G, B, A = 0;
	};
}