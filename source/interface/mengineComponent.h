#pragma once
#include "mengineTypes.h"

// https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
namespace MEngine
{
	class Component // Do NOT inherit from this type directly
	{
	public:
		virtual ~Component() { Destroy(); };

		virtual void Initialize() {};
		virtual void Destroy() {};
	};

	template <class Derived> // Inherit this type for component definitions; Example: class UsefullComponent : public ComponentBase<UsefullComponent>
	class ComponentBase : public Component
	{
	public:
		static void Register(const ComponentBase<Derived>& templateInstance, const char* componentName, uint32_t maxCount = 10)
		{
			ByteSize = sizeof(Derived);
			ComponentMask = MEngine::RegisterComponentType(templateInstance, ByteSize, maxCount, componentName);
		}

		static bool Unregister()
		{
			if (ComponentMask != INVALID_MENGINE_COMPONENT_MASK)
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