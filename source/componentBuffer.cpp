#include "componentBuffer.h"
#include "mengineEntityManagerInternal.h" // TODODB: This is kind of an ugly dependency; see if we can get rid of it
#include <MUtilityLog.h>
#include <MUtilityPlatformDefinitions.h>
#include <cassert>
#include <cstring>
#include <utility>

#define LOG_CATEGORY_COMPONENT_BUFFER "ComponentBuffer"

using namespace MEngine;
using MUtility::Byte;

ComponentBuffer::ComponentBuffer(const Component& templateComponent, uint32_t templateComponentSize, uint32_t startingCapacity, const char* componentName, MEngine::ComponentMask componentMask) :
	m_ComponentByteSize(templateComponentSize), m_Capacity(startingCapacity), ComponentMask(componentMask)
{
	const_cast<Component*>(TemplateComponent) = static_cast<Component*>(malloc(templateComponentSize));
	memcpy(TemplateComponent, &templateComponent, templateComponentSize);

	uint32_t initialByteSize = startingCapacity * m_ComponentByteSize;
	m_Buffer = new Byte[initialByteSize];
	for (uint32_t i = 0; i < startingCapacity; ++i)
	{
		memcpy(m_Buffer + (i * m_ComponentByteSize), TemplateComponent, m_ComponentByteSize); // Copy in the template object to all component in the buffer
	}

	uint32_t nameLength = static_cast<uint32_t>(strlen(componentName));
	ComponentName = new char[nameLength + 1]; // +1 for null terminator
	strcpy(const_cast<char*>(ComponentName), componentName);
}

ComponentBuffer::~ComponentBuffer()
{
	for (uint32_t i = 0; i < GetCount(); ++i)
	{
		reinterpret_cast<Component*>(&(m_Buffer[m_ComponentByteSize * i]))->Destroy();
	}

	delete[] m_Buffer;
	delete[] ComponentName;
	free(TemplateComponent);
}

ComponentBuffer ComponentBuffer::operator=(const ComponentBuffer& other)
{
	return ComponentBuffer(other);
}

uint32_t ComponentBuffer::AllocateComponent(EntityID ownerID)
{
	uint32_t insertIndex = m_NextIndex++;
	m_Owners.push_back(ownerID);
	if (insertIndex == m_Capacity)
		Resize();

	reinterpret_cast<Component*>(&(m_Buffer[m_ComponentByteSize * insertIndex]))->Initialize();
	return insertIndex;
}

bool ComponentBuffer::ReturnComponent(uint32_t componentIndex)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentIndex >= m_NextIndex)
	{
		MLOG_ERROR("Attempted to return component at an inactive index; component name = \"" << ComponentName << '\"', LOG_CATEGORY_COMPONENT_BUFFER);
		return false;
	}
#endif

	reinterpret_cast<Component*>(&(m_Buffer[m_ComponentByteSize * componentIndex]))->Destroy();
	if (componentIndex == m_NextIndex - 1)
	{
		m_Owners.pop_back();
		memcpy(m_Buffer + (m_NextIndex - 1) * m_ComponentByteSize, TemplateComponent, m_ComponentByteSize); // Copy in the templte object to the newly freed position
	}
	else
	{
		EntityID ownerID = m_Owners[componentIndex];
		memcpy(m_Buffer + componentIndex * m_ComponentByteSize, m_Buffer + (m_NextIndex - 1) * m_ComponentByteSize, m_ComponentByteSize); // Copy in the templte object to the newly freed position
		MEngineEntityManager::UpdateComponentIndex(ownerID, ComponentMask, componentIndex); // Tell the owner of the moved component where it has been moved to
		m_Owners.erase(m_Owners.begin() + componentIndex);
	}
	--m_NextIndex;
	return true;
}

Component* ComponentBuffer::GetComponent(uint32_t componentIndex) const
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentIndex >= m_NextIndex)
		MLOG_ERROR("Attempted to get component at an inactive index; component name = \"" << ComponentName << '\"', LOG_CATEGORY_COMPONENT_BUFFER);
#endif
	return reinterpret_cast<Component*>(&(m_Buffer[componentIndex * m_ComponentByteSize]));
}

MUtility::Byte* ComponentBuffer::GetBuffer() const
{
	return m_Buffer;
}

uint32_t ComponentBuffer::GetCount() const
{
	return m_NextIndex;
}

void ComponentBuffer::Resize(uint32_t newCapacity)
{
	if (newCapacity == 0)
		newCapacity = m_Capacity * 2;

	uint32_t oldBufferByteSize = m_Capacity * m_ComponentByteSize;
	uint32_t newBufferByteSize = newCapacity * m_ComponentByteSize;

	Byte* newBuffer = new Byte[newBufferByteSize];
	memcpy(newBuffer, m_Buffer, oldBufferByteSize); // Move the data from the old buffer to the new buffer
	for (uint32_t i = 0; i < newCapacity - m_Capacity; ++i) // Copy in the template object to all new instances
	{
		memcpy(newBuffer + oldBufferByteSize + (i * m_ComponentByteSize), TemplateComponent, m_ComponentByteSize);
	}

	delete[] m_Buffer;
	m_Buffer = newBuffer;
	m_Capacity = newCapacity;
}