#include "Interface/MEngineEntityFactory.h"
#include "Interface/MEngineEntityManager.h"
#include <MUtilityPlatformDefinitions.h>

using namespace MEngine;
using namespace PredefinedColors;

#define LOG_CATEGORY_ENTITY_FACTORY "MEngineEntityFactory"

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

EntityID MEngine::CreateTextBox(int32_t posX, int32_t posY, int32_t width, int32_t height, MEngineFontID fontID, uint32_t posZ, const std::string& text, TextAlignment alignment, TextBoxFlags editFlags, const ColorData& backgroundColor, const ColorData& borderColor)
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

	if ((editFlags & TextBoxFlags::Editable) != 0)
	{
		ButtonComponent* buttonComponent = static_cast<ButtonComponent*>(GetComponentForEntity(ButtonComponent::GetComponentMask(), ID));
		buttonComponent->Callback = new std::function<void()>(std::bind(&TextComponent::StartEditing, *textComponent));
	}

	return ID;
}

bool MEngine::ShowButton(EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if ((GetComponentMask(ID) & BUTTON_ENTITY_MASK) != BUTTON_ENTITY_MASK)
	{
		MLOG_WARNING("Attempted to show button using an EntityID not belonging to a button entity", LOG_CATEGORY_ENTITY_FACTORY);
		return false;
	}
#endif

	static_cast<ButtonComponent*>(MEngine::GetComponentForEntity(ButtonComponent::GetComponentMask(), ID))->IsActive = true;
	static_cast<TextComponent*>(MEngine::GetComponentForEntity(TextComponent::GetComponentMask(), ID))->RenderIgnore = false;
	static_cast<TextureRenderingComponent*>(MEngine::GetComponentForEntity(TextureRenderingComponent::GetComponentMask(), ID))->RenderIgnore = false;

	return true;
}

bool MEngine::HideButton(EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if ((GetComponentMask(ID) & BUTTON_ENTITY_MASK) != BUTTON_ENTITY_MASK)
	{
		MLOG_WARNING("Attempted to hide button using an EntityID not belonging to a button entity", LOG_CATEGORY_ENTITY_FACTORY);
		return false;
	}
#endif

	static_cast<ButtonComponent*>(MEngine::GetComponentForEntity(ButtonComponent::GetComponentMask(), ID))->IsActive = false;
	static_cast<TextComponent*>(MEngine::GetComponentForEntity(TextComponent::GetComponentMask(), ID))->RenderIgnore = true;
	static_cast<TextureRenderingComponent*>(MEngine::GetComponentForEntity(TextureRenderingComponent::GetComponentMask(), ID))->RenderIgnore = true;

	return true;
}

bool MEngine::ShowTextBox(EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if ((GetComponentMask(ID) & TEXT_BOX_ENTITY_MASK) != TEXT_BOX_ENTITY_MASK)
	{
		MLOG_WARNING("Attempted to hide text box using an EntityID not belonging to a text box entity", LOG_CATEGORY_ENTITY_FACTORY);
		return false;
	}
#endif

	static_cast<TextComponent*>(MEngine::GetComponentForEntity(TextComponent::GetComponentMask(), ID))->RenderIgnore = false;
	static_cast<ButtonComponent*>(MEngine::GetComponentForEntity(ButtonComponent::GetComponentMask(), ID))->IsActive = true;
	static_cast<RectangleRenderingComponent*>(MEngine::GetComponentForEntity(RectangleRenderingComponent::GetComponentMask(), ID))->RenderIgnore = false;

	return true;
}

bool MEngine::HideTextBox(EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if ((GetComponentMask(ID) & TEXT_BOX_ENTITY_MASK) != TEXT_BOX_ENTITY_MASK)
	{
		MLOG_WARNING("Attempted to show text box using an EntityID not belonging to a text box entity", LOG_CATEGORY_ENTITY_FACTORY);
		return false;
	}
#endif

	static_cast<TextComponent*>(MEngine::GetComponentForEntity(TextComponent::GetComponentMask(), ID))->RenderIgnore = true;
	static_cast<ButtonComponent*>(MEngine::GetComponentForEntity(ButtonComponent::GetComponentMask(), ID))->IsActive = false;
	static_cast<RectangleRenderingComponent*>(MEngine::GetComponentForEntity(RectangleRenderingComponent::GetComponentMask(), ID))->RenderIgnore = true;

	return true;
}