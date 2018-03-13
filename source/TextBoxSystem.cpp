#include "TextBoxSystem.h"
#include "Interface/MEngineComponentManager.h"
#include "Interface/MEngineEntityFactory.h"
#include "Interface/MEngineEntityManager.h"
#include "Interface/MEngineInternalComponents.h"
#include "Interface/MEngineInput.h"
#include "Interface/MEngineText.h"
#include <vector>

using namespace MEngine;

void TextBoxSystem::UpdatePresentationLayer(float deltaTime)
{
	if (KeyReleased(MKEY_MOUSE_LEFT))
	{
		std::vector<EntityID> textBoxEntites;
		GetEntitiesMatchingMask(TEXT_BOX_ENTITY_MASK, textBoxEntites, MaskMatchMode::Exact);

		bool anyTextBoxPressed = false;
		for (int i = 0; i < textBoxEntites.size(); ++i)
		{
			ButtonComponent* buttonComp = static_cast<ButtonComponent*>(GetComponentForEntity(ButtonComponent::GetComponentMask(), textBoxEntites[i]));
			if (buttonComp->IsTriggered)
			{
				anyTextBoxPressed = true;
				break;
			}
		}

		if (IsTextInputActive() && !anyTextBoxPressed)
		{
			for (int i = 0; i < textBoxEntites.size(); ++i)
			{
				TextComponent* textComp = static_cast<TextComponent*>(GetComponentForEntity(TextComponent::GetComponentMask(), textBoxEntites[i]));
				if (IsInputString(textComp->Text))
				{
					textComp->StopEditing();
					break;
				}
			}
		}
	}
}