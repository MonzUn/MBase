#pragma once
#include "interface/mengineSystem.h"

class ButtonSystem : public MEngine::System
{
	void UpdatePresentationLayer(float deltaTime) override;
};