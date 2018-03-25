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
	std::vector<EntityID> textBoxEntites;
	GetEntitiesMatchingMask(TEXT_BOX_ENTITY_MASK, textBoxEntites);
	bool anyTextBoxPressed = false;
	for (int i = 0; i < textBoxEntites.size(); ++i)
	{
		const PosSizeComponent*	posSizeComp	= static_cast<const PosSizeComponent*>(GetComponentForEntity(PosSizeComponent::GetComponentMask(), textBoxEntites[i]));
		TextComponent*			textComp	= static_cast<TextComponent*>(GetComponentForEntity(TextComponent::GetComponentMask(), textBoxEntites[i]));
		
		if (posSizeComp->IsMouseOver())
		{	
			if ((textComp->EditFlags & TextBoxFlags::Scrollable) != 0)
			{
				PosSizeComponent* posSizeComp = static_cast<PosSizeComponent*>(GetComponentForEntity(PosSizeComponent::GetComponentMask(), textBoxEntites[i]));
				int32_t textHeight = GetTextHeight(textComp->FontID, textComp->Text->c_str());
				if (textHeight > posSizeComp->Height)
				{
					if (ScrolledUp() && textComp->ScrolledLinesCount > 0)
						--textComp->ScrolledLinesCount;
					else if (ScrolledDown() && static_cast<int32_t>(textComp->ScrolledLinesCount) < (static_cast<float>((textHeight - posSizeComp->Height)) / GetLineHeight(textComp->FontID)))
						++textComp->ScrolledLinesCount;
				}
			}

			if ((textComp->EditFlags & TextBoxFlags::Editable) != 0)
			{
				const ButtonComponent* buttonComp = static_cast<const ButtonComponent*>(GetComponentForEntity(ButtonComponent::GetComponentMask(), textBoxEntites[i]));
				if (buttonComp->IsTriggered)
				{
					anyTextBoxPressed = true;
					break;
				}
			}
		}
	}

	if (IsTextInputActive() && KeyReleased(MKEY_MOUSE_LEFT) && !anyTextBoxPressed)
	{
		for (int i = 0; i < textBoxEntites.size(); ++i)
		{
			TextComponent* textComp = static_cast<TextComponent*>(GetComponentForEntity(TextComponent::GetComponentMask(), textBoxEntites[i]));
			if (IsInputString(textComp->Text))
			{
				textComp->StopEditing(); // TODODB: Fix issue that StopEditing isn't executed for the text box being inactivated when pressing anohter textbox
				break;
			}
		}
	}
}