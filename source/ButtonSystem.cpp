#include "ButtonSystem.h"
#include "Interface/MEngineComponentManager.h"
#include "Interface/MEngineEntityManager.h"
#include "Interface/MEngineInternalComponents.h"
#include "Interface/MEngineInput.h"
#include "Interface/MEngineText.h"
#include <vector>

using namespace MEngine;

void ButtonSystem::UpdatePresentationLayer(float deltaTime)
{
	int32_t cursorPosX = GetCursorPosX();
	int32_t cursorPosY = GetCursorPosY();
	
	std::vector<EntityID> entities;
	GetEntitiesMatchingMask(PosSizeComponent::GetComponentMask() | ButtonComponent::GetComponentMask(), entities);
	for (int i = 0; i < entities.size(); ++i)
	{
		bool wasClicked = false;

		ButtonComponent* button = static_cast<ButtonComponent*>(GetComponentForEntity(ButtonComponent::GetComponentMask(), entities[i])); // TODODB: Rename buttonComp
		const PosSizeComponent* posSizeComponent = static_cast<const PosSizeComponent*>(GetComponentForEntity(PosSizeComponent::GetComponentMask(), entities[i]));

		if (button->Callback != nullptr && button->IsActive)
		{
			button->IsMouseOver = posSizeComponent->IsMouseOver();
			if (button->IsMouseOver)
			{
				// TODODB: Tint the button when it is hovered
				if (KeyReleased(MKEY_MOUSE_LEFT))
				{
					button->Callback->operator()();
					wasClicked = true;
				}
			}
			button->IsTriggered = wasClicked;
		}
	}
}