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