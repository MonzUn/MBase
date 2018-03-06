#include "textBoxSystem.h"
#include "interface/mengineComponentManager.h"
#include "interface/mengineEntityFactory.h"
#include "interface/mengineEntityManager.h"
#include "interface/mengineInternalComponents.h"
#include "interface/mengineInput.h"
#include "interface/mengineText.h"
#include <vector>

using namespace MEngine;

void TextBoxSystem::UpdatePresentationLayer(float deltaTime)
{
	int32_t componentCount = -1;
	TextBoxComponent* textBoxes = reinterpret_cast<TextBoxComponent*>(GetComponentBuffer(TextBoxComponent::GetComponentMask(), &componentCount));
	for (int i = 0; i < componentCount; ++i)
	{
		const TextBoxComponent& textBox = textBoxes[i];
		if (IsInputString(textBox.Text))
			DrawTextWithCaret(textBox.PosX, textBox.PosY + textBox.Height - GetTextHeight(textBox.Text->c_str()), *textBox.Text);
		else
			DrawText(textBox.PosX, textBox.PosY + textBox.Height - GetTextHeight(textBox.Text->c_str()), *textBox.Text);
	}

	if (KeyReleased(MKEY_MOUSE_LEFT))
	{
		std::vector<EntityID> textBoxEntites;
		GetEntitiesMatchingMask(TEXT_BOX_ENTITY_MASK, textBoxEntites, true);

		bool anyTextBoxPressed = false;
		for (int i = 0; i < textBoxEntites.size(); ++i)
		{
			ButtonComponent* button = static_cast<ButtonComponent*>(GetComponentForEntity(ButtonComponent::GetComponentMask(), textBoxEntites[i]));
			if (button->IsTriggered)
			{
				anyTextBoxPressed = true;
				break;
			}
		}

		if (IsTextInputActive() && !anyTextBoxPressed)
			StopTextInput();
	}
}