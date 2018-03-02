#include "interface/mengineEntityFactory.h"
#include "interface/mengineEntityManager.h"

MEngine::EntityID MEngine::CreateButton(int32_t posX, int32_t posY, int32_t width, int32_t height, std::function<void()> callback, TextureID texture, const std::string& text)
{
	EntityID ID = CreateEntity();
	ComponentMask combinedMask = ButtonComponent::GetComponentMask() | TextureRenderingComponent::GetComponentMask();
	AddComponentsToEntity(combinedMask, ID);

	ButtonComponent* buttonComponent = static_cast<ButtonComponent*>(GetComponentForEntity(ButtonComponent::GetComponentMask(), ID));
	buttonComponent->PosX		= posX;
	buttonComponent->PosY		= posY;
	buttonComponent->Width		= width;
	buttonComponent->Height		= height;
	buttonComponent->Callback	= callback;
	buttonComponent->text		= text;

	TextureRenderingComponent* textureComponent = static_cast<TextureRenderingComponent*>(GetComponentForEntity(TextureRenderingComponent::GetComponentMask(), ID));
	textureComponent->PosX = posX;
	textureComponent->PosY = posY;
	textureComponent->Width = width;
	textureComponent->Height = height;
	textureComponent->TextureID = texture;

	return ID;
}