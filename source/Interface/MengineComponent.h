#pragma once
#include "MEngineTypes.h"

// https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
namespace MEngine
{
	class Component // Do NOT inherit from this type directly
	{
	public:
		virtual ~Component() {};

		virtual void Initialize() = 0;
		virtual void Destroy() = 0;
	};

	template <class Derived> // Inherit this type for component definitions; Example: class UsefullComponent : public ComponentBase<UsefullComponent>
	class ComponentBase : public Component
	{
	public:
		virtual void Initialize() {};
		virtual void Destroy() {};

		static void Register(const ComponentBase<Derived>& templateInstance, const char* componentName, uint32_t maxCount = 10)
		{
			ByteSize = sizeof(Derived);
			ComponentMask = MEngine::RegisterComponentType(templateInstance, ByteSize, maxCount, componentName);
		}

		static bool Unregister()
		{
			if (ComponentMask != MENGINE_INVALID_COMPONENT_MASK)
				return MEngine::UnregisterComponentType(ComponentMask);

			return false;
		}

		static ComponentMask GetComponentMask() { return ComponentMask; }
		static uint32_t GetByteSize() { return ByteSize; }

	private:
		static ComponentMask ComponentMask;
		static uint32_t ByteSize;
	};
	template <class Derived> ComponentMask ComponentBase<Derived>::ComponentMask;
	template <class Derived> uint32_t ComponentBase<Derived>::ByteSize;
}