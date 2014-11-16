#include "MetaObject.h"
#include <algorithm>
#include <iostream>

namespace orm
{

pkMetaObject::pkMetaObject(const StructInfoDescriptor *descriptor)
	: m_descriptor(descriptor), m_initialized(false)
{
}

pkMetaObject::~pkMetaObject(void)
{
}

void pkMetaObject::preparseFields() const
{
	if (m_initialized) return;
	for (const StructInfoDescriptor *desc = m_descriptor; desc; desc = desc->m_superDescriptor)
	{
		for (size_t i = 0; desc->m_fieldDescriptors[i].m_pszName; ++i) 
		{
			//std::cout << desc->m_fieldDescriptors[i].m_pszName << std::endl;
			m_fields.push_back(&desc->m_fieldDescriptors[i]);
		}
	}
	std::sort(m_fields.begin(), m_fields.end(), [](const FieldInfoDescriptor *a, const FieldInfoDescriptor *b) { return a->m_dwOffset < b->m_dwOffset; });
	m_initialized = true;
}

} // namespace pkapi
