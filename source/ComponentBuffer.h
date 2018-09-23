#pragma once
#include "Interface/MEngineComponent.h"
#include "Interface/MEngineTypes.h"
#include <MUtilityByte.h>
#include <MUtilityIDBank.h>
#include <stdint.h>
#include <queue>

namespace MEngine
{
	typedef MUtility::MUtilityIDBank<uint32_t> ComponentIDBank;

	class ComponentBuffer
	{
	public:
		ComponentBuffer(const Component& templateComponent, uint32_t templateComponentSize, uint32_t startingCapacity, const char* componentName, ComponentMask componentMask);
		ComponentBuffer(const ComponentBuffer& other) = default;
		~ComponentBuffer();

		ComponentBuffer operator=(const ComponentBuffer& other);

		uint32_t AllocateComponent(EntityID ownerID); // Returns true if a resize occured
		bool ReturnComponent(uint32_t componentIndex);

		Component* GetComponent(uint32_t componentIndex) const;
		MUtility::Byte* GetBuffer() const;
		const ComponentIDBank& GetIDs() const;
		uint32_t GetTotalCount() const;
		uint32_t GetActiveCount() const;

		void Resize(uint32_t newCapacity = 0); // newCapacity = 0 will double the capacity

		const ComponentMask			ComponentType		= MENGINE_INVALID_COMPONENT_MASK;
		const char*					ComponentName		= nullptr;
		Component*					TemplateComponent	= nullptr;

	private:
		const uint32_t	m_ComponentByteSize = 0;

		MUtility::Byte*	m_Buffer	= nullptr;
		uint32_t		m_Capacity	= 0;
		ComponentIDBank	m_IDs;
	};
}