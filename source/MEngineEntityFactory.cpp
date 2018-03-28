#include "Interface/MEngineEntityFactory.h"
#include "Interface/MEngineEntityManager.h"
#include <MUtilityPlatformDefinitions.h>

using namespace MEngine;
using namespace PredefinedColors;

#define LOG_CATEGORY_ENTITY_FACTORY "MEngineEntityFactory"

EntityID MEngine::CreateButton(int32_t posX, int32_t posY, int32_t width, int32_t height, std::function<void()> callback, uint32_t posZ, TextureID texture, MEngineFontID fontID, const std::string& text, TextAlignment textAlignment)
{
	EntityID ID = CreateEntity();
	AddComponentsToEntity(ID, BUTTON_ENTITY_MASK);

	PosSizeComponent* posSizeComponent = static_cast<PosSizeComponent*>(GetComponent(ID, PosSizeComponent::GetComponentMask()));
	posSizeComponent->PosX = posX;
	posSizeComponent->PosY = posY;
	posSizeComponent->PosZ = posZ;
	posSizeComponent->Width = width;
	posSizeComponent->Height = height;

	ButtonComponent* buttonComponent = static_cast<ButtonComponent*>(GetComponent(ID, ButtonComponent::GetComponentMask()));
	buttonComponent->Callback	= new std::function<void()>(callback);

	TextureRenderingComponent* textureComponent = static_cast<TextureRenderingComponent*>(GetComponent(ID, TextureRenderingComponent::GetComponentMask()));
	textureComponent->TextureID = texture;
	if (texture == INVALID_MENGINE_TEXTURE_ID)
		textureComponent->RenderIgnore = true;

	TextComponent* textComponent = static_cast<TextComponent*>(GetComponent(ID, TextComponent::GetComponentMask()));
	textComponent->Text = new std::string(text);
	textComponent->DefaultText = new std::string(text);
	textComponent->FontID = fontID;
	textComponent->Alignment = textAlignment;

	return ID;
}

EntityID MEngine::CreateTextBox(int32_t posX, int32_t posY, int32_t width, int32_t height, MEngineFontID fontID, uint32_t posZ, const std::string& text, TextAlignment alignment, TextBoxFlags editFlags, const ColorData& backgroundColor, const ColorData& borderColor)
{
	EntityID ID = CreateEntity();
	ComponentMask entityMask = (editFlags & TextBoxFlags::Editable) != 0 ? TEXT_BOX_EDITABLE_ENTITY_MASK : TEXT_BOX_ENTITY_MASK;
	AddComponentsToEntity(ID, entityMask);

	PosSizeComponent* posSizeComponent = static_cast<PosSizeComponent*>(GetComponent(ID, PosSizeComponent::GetComponentMask()));
	posSizeComponent->PosX		= posX;
	posSizeComponent->PosY		= posY;
	posSizeComponent->PosZ		= posZ;
	posSizeComponent->Width		= width;
	posSizeComponent->Height	= height;

	RectangleRenderingComponent* rectangleComponent = static_cast<RectangleRenderingComponent*>(GetComponent(ID, RectangleRenderingComponent::GetComponentMask()));
	rectangleComponent->FillColor	= backgroundColor;
	rectangleComponent->BorderColor = borderColor;

	TextComponent* textComponent	= static_cast<TextComponent*>(GetComponent(ID, TextComponent::GetComponentMask()));
	textComponent->Text				= new std::string(text);
	textComponent->DefaultText		= new std::string(text);
	textComponent->FontID			= fontID;
	textComponent->Alignment		= alignment;
	textComponent->EditFlags		= editFlags;

	if ((editFlags & TextBoxFlags::Editable) != 0)
	{
		ButtonComponent* buttonComponent = static_cast<ButtonComponent*>(GetComponent(ID, ButtonComponent::GetComponentMask()));
		buttonComponent->Callback = new std::function<void()>(std::bind(&TextComponent::StartEditing, *textComponent));
	}

	return ID;
}

// TODODB: If a button is inactive -> gets hidden -> gets shown; the button will no longer be inactive. Needs a fix
bool MEngine::ShowButton(EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if ((GetComponentMask(ID) & BUTTON_ENTITY_MASK) != BUTTON_ENTITY_MASK)
	{
		MLOG_WARNING("Attempted to show button using an EntityID not belonging to a button entity", LOG_CATEGORY_ENTITY_FACTORY);
		return false;
	}
#endif
	TextureRenderingComponent* textureComp = static_cast<TextureRenderingComponent*>(MEngine::GetComponent(ID, TextureRenderingComponent::GetComponentMask()));
	if (textureComp->RenderIgnore)
	{
		static_cast<ButtonComponent*>(MEngine::GetComponent(ID, ButtonComponent::GetComponentMask()))->IsActive = true;
		static_cast<TextComponent*>(MEngine::GetComponent(ID, TextComponent::GetComponentMask()))->RenderIgnore = false;
		textureComp->RenderIgnore = false;
	}

	return !textureComp->RenderIgnore;
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
	TextureRenderingComponent* textureComp = static_cast<TextureRenderingComponent*>(MEngine::GetComponent(ID, TextureRenderingComponent::GetComponentMask()));
	if (!textureComp->RenderIgnore)
	{
		static_cast<ButtonComponent*>(MEngine::GetComponent(ID, ButtonComponent::GetComponentMask()))->IsActive = false;
		static_cast<TextComponent*>(MEngine::GetComponent(ID, TextComponent::GetComponentMask()))->RenderIgnore = true;
		textureComp->RenderIgnore = true;
	}
	return textureComp->RenderIgnore;
}

bool MEngine::ShowTextBox(EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if ((GetComponentMask(ID) & TEXT_BOX_EDITABLE_ENTITY_MASK) != TEXT_BOX_EDITABLE_ENTITY_MASK && (GetComponentMask(ID) & TEXT_BOX_ENTITY_MASK) != TEXT_BOX_ENTITY_MASK)
	{
		MLOG_WARNING("Attempted to hide text box using an EntityID not belonging to a text box entity", LOG_CATEGORY_ENTITY_FACTORY);
		return false;
	}
#endif
	TextComponent* textComp = static_cast<TextComponent*>(MEngine::GetComponent(ID, TextComponent::GetComponentMask()));
	if (textComp->RenderIgnore)
	{
		textComp->RenderIgnore = false;
		static_cast<RectangleRenderingComponent*>(MEngine::GetComponent(ID, RectangleRenderingComponent::GetComponentMask()))->RenderIgnore = false;
		if ((GetComponentMask(ID) & TEXT_BOX_EDITABLE_ENTITY_MASK) == TEXT_BOX_EDITABLE_ENTITY_MASK)
			static_cast<ButtonComponent*>(MEngine::GetComponent(ID, ButtonComponent::GetComponentMask()))->IsActive = true;
	}
	return !textComp->RenderIgnore;
}

bool MEngine::HideTextBox(EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if ((GetComponentMask(ID) & TEXT_BOX_EDITABLE_ENTITY_MASK) != TEXT_BOX_EDITABLE_ENTITY_MASK && (GetComponentMask(ID) & TEXT_BOX_ENTITY_MASK) != TEXT_BOX_ENTITY_MASK)
	{
		MLOG_WARNING("Attempted to show text box using an EntityID not belonging to a text box entity", LOG_CATEGORY_ENTITY_FACTORY);
		return false;
	}
#endif

	TextComponent* textComp = static_cast<TextComponent*>(MEngine::GetComponent(ID, TextComponent::GetComponentMask()));
	if (!textComp->RenderIgnore)
	{
		textComp->RenderIgnore = true;
		static_cast<RectangleRenderingComponent*>(MEngine::GetComponent(ID, RectangleRenderingComponent::GetComponentMask()))->RenderIgnore = true;
		if ((GetComponentMask(ID) & TEXT_BOX_EDITABLE_ENTITY_MASK) == TEXT_BOX_EDITABLE_ENTITY_MASK)
			static_cast<ButtonComponent*>(MEngine::GetComponent(ID, ButtonComponent::GetComponentMask()))->IsActive = false;
	}
	return textComp->RenderIgnore;
}