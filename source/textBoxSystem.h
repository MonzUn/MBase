#pragma once
#include "interface/mengineSystem.h"

class TextBoxSystem : public MEngine::System
{
	void UpdatePresentationLayer(float deltaTime) override;
};