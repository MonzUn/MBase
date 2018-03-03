#pragma once
#include <stdint.h>

namespace MEngine
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

	struct ColorData
	{
		ColorData() { R = G = B = A = 0; }

		ColorData(PredefinedColors::PredefinedColorEnum color) :
			R(PredefinedColors::ColorValues[color][0]), G(PredefinedColors::ColorValues[color][1]),
			B(PredefinedColors::ColorValues[color][2]), A(PredefinedColors::ColorValues[color][3]) {}
		ColorData(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) :
			R(red), G(green), B(blue), A(alpha) {}

		uint8_t R, G, B, A = 0;

		bool IsFullyTransparent() const
		{
			return A == 0;
		}

		bool operator==(const ColorData& other) const
		{
			return R == other.R && G == other.G && B == other.B && A == other.A;
		}

		bool operator==(const uint8_t otherRGBA[4]) const
		{
			return R == otherRGBA[0] && G == otherRGBA[1] && B == otherRGBA[2] && A == otherRGBA[3];
		}
	};
}