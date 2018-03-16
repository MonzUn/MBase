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

		ButtonComponent* button = static_cast<ButtonComponent*>(GetComponentForEntity(ButtonComponent::GetComponentMask(), entities[i]));
		const PosSizeComponent* posSizeComponent = static_cast<const PosSizeComponent*>(GetComponentForEntity(PosSizeComponent::GetComponentMask(), entities[i]));

		button->IsMouseOver = button->IsActive && cursorPosX >= posSizeComponent->PosX && cursorPosX < posSizeComponent->PosX + posSizeComponent->Width && cursorPosY >= posSizeComponent->PosY && cursorPosY < posSizeComponent->PosY + posSizeComponent->Height;
		if (button->Callback != nullptr && button->IsMouseOver)
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