#include "buttonSystem.h"
#include "interface/mengineComponentManager.h"
#include "interface/mengineInternalComponents.h"
#include "interface/mengineInput.h"
#include "interface/mengineText.h"

using namespace MEngine;

void ButtonSystem::UpdatePresentationLayer(float deltaTime)
{
	int32_t cursorPosX = GetCursorPosX();
	int32_t cursorPosY = GetCursorPosY();
	int32_t componentCount = -1;
	ButtonComponent* buttons = reinterpret_cast<ButtonComponent*>(GetComponentBuffer(ButtonComponent::GetComponentMask(), &componentCount));
	for (int i = 0; i < componentCount; ++i)
	{
		bool wasClicked = false;
		ButtonComponent& button = buttons[i];
		if (button.Callback != nullptr && cursorPosX >= button.PosX && cursorPosX < button.PosX + button.Width && cursorPosY >= button.PosY && cursorPosY < button.PosY + button.Height)
		{
			// TODODB: Tint the button when it is hovered
			if (KeyReleased(MKEY_MOUSE_LEFT))
			{
				button.Callback->operator()();
				wasClicked = true;
			}
		}
		button.IsTriggered = wasClicked;

		if(button.Text != nullptr)
			MEngine::DrawText(button.PosX + (button.Width / 2) - (GetTextWidth(button.Text->c_str()) / 2), button.PosY + (button.Height / 2) - (GetTextHeight(button.Text->c_str()) / 2), *button.Text); // Centered
	}
}