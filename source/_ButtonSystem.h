#pragma once
#include "Interface/MEngineSystem.h"

class ButtonSystem : public MEngine::System
{
	void UpdatePresentationLayer(float deltaTime) override;
};