#include "interface/mengineEntityFactory.h"
#include "interface/mengineEntityManager.h"

using namespace MEngine;
using namespace PredefinedColors;

EntityID MEngine::CreateButton(int32_t posX, int32_t posY, int32_t width, int32_t height, std::function<void()> callback, TextureID texture, const std::string& text)
{
	EntityID ID = CreateEntity();
	AddComponentsToEntity(BUTTON_ENTITY_MASK, ID);

	PosSizeComponent* posSizeComponent = static_cast<PosSizeComponent*>(GetComponentForEntity(PosSizeComponent::GetComponentMask(), ID));
	posSizeComponent->PosX = posX;
	posSizeComponent->PosY = posY;
	posSizeComponent->Width = width;
	posSizeComponent->Height = height;

	ButtonComponent* buttonComponent = static_cast<ButtonComponent*>(GetComponentForEntity(ButtonComponent::GetComponentMask(), ID));
	buttonComponent->Callback	= new std::function<void()>(callback);

	TextureRenderingComponent* textureComponent = static_cast<TextureRenderingComponent*>(GetComponentForEntity(TextureRenderingComponent::GetComponentMask(), ID));
	textureComponent->TextureID = texture;
	if (texture == INVALID_MENGINE_TEXTURE_ID)
		textureComponent->RenderIgnore = true;

	TextComponent* textComponent = static_cast<TextComponent*>(GetComponentForEntity(TextComponent::GetComponentMask(), ID));
	textComponent->Text = new std::string(text);

	return ID;
}

EntityID MEngine::CreateTextBox(int32_t posX, int32_t posY, int32_t width, int32_t height, bool editable, const std::string& text, const ColorData& backgroundColor, const ColorData& borderColor)
{
	EntityID ID = CreateEntity();
	AddComponentsToEntity(TEXT_BOX_ENTITY_MASK, ID);

	PosSizeComponent* posSizeComponent = static_cast<PosSizeComponent*>(GetComponentForEntity(PosSizeComponent::GetComponentMask(), ID));
	posSizeComponent->PosX		= posX;
	posSizeComponent->PosY		= posY;
	posSizeComponent->Width		= width;
	posSizeComponent->Height	= height;

	RectangleRenderingComponent* rectangleComponent = static_cast<RectangleRenderingComponent*>(GetComponentForEntity(RectangleRenderingComponent::GetComponentMask(), ID));
	rectangleComponent->FillColor	= backgroundColor;
	rectangleComponent->BorderColor = borderColor;

	TextComponent* textComponent = static_cast<TextComponent*>(GetComponentForEntity(TextComponent::GetComponentMask(), ID));
	textComponent->Text	= new std::string(text);

	if (editable)
	{
		ButtonComponent* buttonComponent = static_cast<ButtonComponent*>(GetComponentForEntity(ButtonComponent::GetComponentMask(), ID));
		buttonComponent->Callback = new std::function<void()>(std::bind(&TextComponent::StartEditing, textComponent));
	}

	return ID;
}