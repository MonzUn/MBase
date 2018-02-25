#pragma once
#include "interface/mengineComponent.h"
#include "interface/mengineTypes.h"
#include <MUtilityByte.h>
#include <MUtilityIDBank.h>
#include <stdint.h>
#include <queue>

namespace MEngine
{
	class ComponentBuffer // TODODB: Add support for component destruction
	{
	public:
		ComponentBuffer(const Component& templateComponent, uint32_t templateComponentSize, uint32_t startingCapacity, const char* componentName, MEngineComponentMask componentMask);
		~ComponentBuffer();

		ComponentBuffer operator=(const ComponentBuffer& other);

		uint32_t AllocateComponent(MEngineEntityID ownerID); // Returns true if a resize occured
		bool ReturnComponent(uint32_t componentIndex);

		Component* GetComponent(uint32_t componentIndex) const;
		MUtility::Byte* GetBuffer() const;
		uint32_t GetCount() const;

		void Resize(uint32_t newCapacity = 0); // newCapacity = 0 will double the capacity

		const MEngineComponentMask	ComponentMask		= INVALID_MENGINE_COMPONENT_MASK;
		const char*					ComponentName		= nullptr;
		Component*					TemplateComponent	= nullptr;

	private:
		const uint32_t				m_ComponentByteSize = 0;

		MUtility::Byte*	m_Buffer	= nullptr;
		uint32_t		m_Capacity	= 0;
		uint32_t		m_NextIndex = 0;
		std::vector<MEngineEntityID> m_Owners;
	};
}