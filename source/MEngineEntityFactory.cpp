#include "Interface/MEngineEntityFactory.h"
#include "Interface/MEngineEntityManager.h"

using namespace MEngine;
using namespace PredefinedColors;

EntityID MEngine::CreateButton(int32_t posX, int32_t posY, int32_t width, int32_t height, std::function<void()> callback, uint32_t posZ, TextureID texture, MEngineFontID fontID, const std::string& text, TextAlignment textAlignment)
{
	EntityID ID = CreateEntity();
	AddComponentsToEntity(BUTTON_ENTITY_MASK, ID);

	PosSizeComponent* posSizeComponent = static_cast<PosSizeComponent*>(GetComponentForEntity(PosSizeComponent::GetComponentMask(), ID));
	posSizeComponent->PosX = posX;
	posSizeComponent->PosY = posY;
	posSizeComponent->PosZ = posZ;
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
	textComponent->DefaultText = new std::string(text);
	textComponent->FontID = fontID;
	textComponent->Alignment = textAlignment;

	return ID;
}

EntityID MEngine::CreateTextBox(int32_t posX, int32_t posY, int32_t width, int32_t height, MEngineFontID fontID, uint32_t posZ, const std::string& text, TextAlignment alignment, TextBoxEditFlags editFlags, const ColorData& backgroundColor, const ColorData& borderColor)
{
	EntityID ID = CreateEntity();
	AddComponentsToEntity(TEXT_BOX_ENTITY_MASK, ID);

	PosSizeComponent* posSizeComponent = static_cast<PosSizeComponent*>(GetComponentForEntity(PosSizeComponent::GetComponentMask(), ID));
	posSizeComponent->PosX		= posX;
	posSizeComponent->PosY		= posY;
	posSizeComponent->PosZ		= posZ;
	posSizeComponent->Width		= width;
	posSizeComponent->Height	= height;

	RectangleRenderingComponent* rectangleComponent = static_cast<RectangleRenderingComponent*>(GetComponentForEntity(RectangleRenderingComponent::GetComponentMask(), ID));
	rectangleComponent->FillColor	= backgroundColor;
	rectangleComponent->BorderColor = borderColor;

	TextComponent* textComponent	= static_cast<TextComponent*>(GetComponentForEntity(TextComponent::GetComponentMask(), ID));
	textComponent->Text				= new std::string(text);
	textComponent->DefaultText		= new std::string(text);
	textComponent->FontID			= fontID;
	textComponent->Alignment		= alignment;
	textComponent->EditFlags		= editFlags;

	if ((editFlags & TextBoxEditFlags::Editable) != 0)
	{
		ButtonComponent* buttonComponent = static_cast<ButtonComponent*>(GetComponentForEntity(ButtonComponent::GetComponentMask(), ID));
		buttonComponent->Callback = new std::function<void()>(std::bind(&TextComponent::StartEditing, textComponent));
	}

	return ID;
}