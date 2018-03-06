#include "buttonSystem.h"
#include "interface/mengineComponentManager.h"
#include "interface/mengineEntityManager.h"
#include "interface/mengineInternalComponents.h"
#include "interface/mengineInput.h"
#include "interface/mengineText.h"
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
		PosSizeComponent* posSizeComponent = static_cast<PosSizeComponent*>(GetComponentForEntity(PosSizeComponent::GetComponentMask(), entities[i]));
		ButtonComponent* button = static_cast<ButtonComponent*>(GetComponentForEntity(ButtonComponent::GetComponentMask(), entities[i]));

		if (button->Callback != nullptr && cursorPosX >= posSizeComponent->PosX && cursorPosX < posSizeComponent->PosX + posSizeComponent->Width && cursorPosY >= posSizeComponent->PosY && cursorPosY < posSizeComponent->PosY + posSizeComponent->Height)
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