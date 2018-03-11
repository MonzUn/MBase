#pragma once
#include "Interface/MEngineSystem.h"

class TextBoxSystem : public MEngine::System
{
	void UpdatePresentationLayer(float deltaTime) override;
};