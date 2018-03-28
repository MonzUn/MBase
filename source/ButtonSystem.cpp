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

		ButtonComponent* buttonComp = static_cast<ButtonComponent*>(GetComponent(entities[i], ButtonComponent::GetComponentMask()));
		const PosSizeComponent* posSizeComp = static_cast<const PosSizeComponent*>(GetComponent(entities[i], PosSizeComponent::GetComponentMask()));

		if (buttonComp->Callback != nullptr && buttonComp->IsActive)
		{
			buttonComp->IsMouseOver = posSizeComp->IsMouseOver();
			if (buttonComp->IsMouseOver)
			{
				// TODODB: Tint the button when it is hovered
				if (KeyReleased(MKEY_MOUSE_LEFT))
				{
					buttonComp->Callback->operator()();
					wasClicked = true;
				}
			}
			buttonComp->IsTriggered = wasClicked;
		}
	}
}