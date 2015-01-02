#include "MetaObject.h"
#include <algorithm>
#include <iostream>
#include <mutex>

namespace metacpp
{

MetaObject::MetaObject(const StructInfoDescriptor *descriptor)
	: m_descriptor(descriptor), m_initialized(false)
{
}

MetaObject::~MetaObject(void)
{
}

const char *MetaObject::name() const
{
    return m_descriptor->m_strucName;
}

const MetaField *MetaObject::field(size_t i) const
{
    preparseFields();
    return m_fields[i].get();
}

const MetaField *MetaObject::fieldByOffset(ptrdiff_t offset) const
{
    preparseFields();
    auto it = std::lower_bound(m_fields.begin(), m_fields.end(), offset,
        [](const std::unique_ptr<MetaField>& field, ptrdiff_t off) -> bool
        {
            return field->offset() < off;
        }
    );
    return it ==  m_fields.end() ? nullptr : it->get();
}

const MetaField *MetaObject::fieldByName(const String &name, bool caseSensetive) const
{
    preparseFields();
    auto it = std::find_if(m_fields.begin(), m_fields.end(),
        [=](const std::unique_ptr<MetaField>& field)
        {
            return name.equals(field->name(), caseSensetive);
        });
    return it == m_fields.end() ? nullptr : it->get();
}

size_t MetaObject::totalFields() const
{
    preparseFields();
    return m_fields.size();
}

void MetaObject::preparseFields() const
{
    static std::mutex _mutex;
    std::lock_guard<std::mutex> _guard(_mutex);

	if (m_initialized) return;
    MetaFieldFactory factory;
	for (const StructInfoDescriptor *desc = m_descriptor; desc; desc = desc->m_superDescriptor)
	{
		for (size_t i = 0; desc->m_fieldDescriptors[i].m_pszName; ++i) 
		{
            m_fields.push_back(std::move(factory.createInstance(desc->m_fieldDescriptors + i)));
		}
	}
    std::sort(m_fields.begin(), m_fields.end(), [](const std::unique_ptr<MetaField>& a, const std::unique_ptr<MetaField>& b)
        {return a->offset() < b->offset(); });
	m_initialized = true;
}


MetaField::MetaField(const FieldInfoDescriptor *fieldDescriptor)
    : m_descriptor(fieldDescriptor)
{

}

MetaField::~MetaField()
{
}

const char *MetaField::name() const
{
    return m_descriptor->m_pszName;
}

size_t MetaField::size() const
{
    return m_descriptor->m_dwSize;
}

ptrdiff_t MetaField::offset() const
{
    return m_descriptor->m_dwOffset;
}

EFieldType MetaField::type() const
{
    return m_descriptor->m_eType;
}

bool MetaField::nullable() const
{
    return m_descriptor->m_nullable;
}

EMandatoriness MetaField::mandatoriness() const
{
    return m_descriptor->valueInfo.mandatoriness;
}

std::unique_ptr<MetaField> MetaFieldFactory::createInstance(const FieldInfoDescriptor *arg)
{
    std::unique_ptr<MetaField> result;
    switch (arg->m_eType)
    {
    case eFieldBool:
        result.reset(new MetaFieldBool(arg));
        break;
    case eFieldInt:
        result.reset(new MetaFieldInt(arg));
        break;
    case eFieldUint:
        result.reset(new MetaFieldUint(arg));
        break;
    case eFieldFloat:
        result.reset(new MetaFieldFloat(arg));
        break;
    case eFieldString:
        result.reset(new MetaFieldString(arg));
        break;
    case eFieldEnum:
        result.reset(new MetaFieldEnum(arg));
        break;
    case eFieldObject:
        result.reset(new MetaFieldObject(arg));
        break;
    case eFieldArray:
        result.reset(new MetaFieldArray(arg));
        break;
    case eFieldTime:
        result.reset(new MetaFieldTime(arg));
        break;
    }
    return result;
}

} // namespace metacpp
