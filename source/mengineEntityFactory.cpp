#include "interface/mengineEntityFactory.h"
#include "interface/mengineEntityManager.h"

using namespace MEngine;

EntityID MEngine::CreateButton(int32_t posX, int32_t posY, int32_t width, int32_t height, std::function<void()> callback, TextureID texture, const std::string& text)
{
	EntityID ID = CreateEntity();
	AddComponentsToEntity(BUTTON_ENTITY_MASK, ID);

	ButtonComponent* buttonComponent = static_cast<ButtonComponent*>(GetComponentForEntity(ButtonComponent::GetComponentMask(), ID));
	buttonComponent->PosX		= posX;
	buttonComponent->PosY		= posY;
	buttonComponent->Width		= width;
	buttonComponent->Height		= height;
	buttonComponent->Callback	= new std::function<void()>(callback);
	buttonComponent->text		= new std::string(text);

	TextureRenderingComponent* textureComponent = static_cast<TextureRenderingComponent*>(GetComponentForEntity(TextureRenderingComponent::GetComponentMask(), ID));
	textureComponent->PosX		= posX;
	textureComponent->PosY		= posY;
	textureComponent->Width		= width;
	textureComponent->Height	= height;
	textureComponent->TextureID = texture;
	if (texture == INVALID_MENGINE_TEXTURE_ID)
		textureComponent->RenderIgnore = true;

	return ID;
}

	return ID;
}