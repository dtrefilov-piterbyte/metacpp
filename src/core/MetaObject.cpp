/****************************************************************************
* Copyright 2014-2015 Trefilov Dmitrij                                      *
*                                                                           *
* Licensed under the Apache License, Version 2.0 (the "License");           *
* you may not use this file except in compliance with the License.          *
* You may obtain a copy of the License at                                   *
*                                                                           *
*    http://www.apache.org/licenses/LICENSE-2.0                             *
*                                                                           *
* Unless required by applicable law or agreed to in writing, software       *
* distributed under the License is distributed on an "AS IS" BASIS,         *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
* See the License for the specific language governing permissions and       *
* limitations under the License.                                            *
****************************************************************************/
#include "MetaObject.h"
#include <algorithm>
#include <iostream>
#include <mutex>

namespace metacpp
{

MetaObject::MetaObject(const MetaInfoDescriptor *descriptor,
                       Object *(*constructor)(void *), void (*destructor)(void *))
    : m_descriptor(descriptor), m_initialized(false),
      m_constructor(constructor), m_destructor(destructor)
{
}

MetaObject::~MetaObject(void)
{
}

const char *MetaObject::name() const
{
    return m_descriptor->m_strucName;
}

const MetaFieldBase *MetaObject::field(size_t i) const
{
    prepare();
    return m_fields[i].get();
}

const MetaFieldBase *MetaObject::fieldByOffset(ptrdiff_t offset) const
{
    prepare();
    auto it = std::lower_bound(m_fields.begin(), m_fields.end(), offset,
        [](const std::unique_ptr<MetaFieldBase>& field, ptrdiff_t off) -> bool
        {
            return field->offset() < off;
        }
    );
    return it ==  m_fields.end() ? nullptr : it->get();
}

const MetaFieldBase *MetaObject::fieldByName(const String &name, bool caseSensetive) const
{
    prepare();
    auto it = std::find_if(m_fields.begin(), m_fields.end(),
        [=](const std::unique_ptr<MetaFieldBase>& field)
        {
            return name.equals(field->name(), caseSensetive);
        });
    return it == m_fields.end() ? nullptr : it->get();
}

size_t MetaObject::totalFields() const
{
    prepare();
    return m_fields.size();
}

const MetaCallBase *MetaObject::method(size_t i) const
{
    prepare();
    return m_methods[i].get();
}

const MetaCallBase *MetaObject::methodByName(const String &name, bool caseSensetive) const
{
    prepare();
    auto it = std::find_if(m_methods.begin(), m_methods.end(),
        [=](const std::unique_ptr<MetaCallBase>& method)
        {
            return name.equals(method->name(), caseSensetive);
        });
    return it == m_methods.end() ? nullptr : it->get();
}

size_t MetaObject::totalMethods() const
{
    prepare();
    return m_methods.size();
}

const MetaObject *MetaObject::superMetaObject() const
{
    if (!m_descriptor->m_superDescriptor) return nullptr;
    if (!m_super)
        m_super.reset(new MetaObject(m_descriptor->m_superDescriptor));
    return m_super.get();
}

size_t MetaObject::size() const
{
    return m_descriptor->m_dwSize;
}

Object *MetaObject::createInstance() const
{
    if (!m_constructor)
        throw std::runtime_error("Have no appropriate class constructor");
    void *pMem = ::operator new(size());
    return m_constructor(pMem);
}

void MetaObject::destroyInstance(Object *object) const
{
    if (!m_destructor)
        throw std::runtime_error("Have no appropriate class destructor");
    m_destructor(object);
    ::operator delete(object);
}

Variant MetaObject::invoke(const String &methodName, const VariantArray &args) const
{
    prepare();
    for (auto& method : m_methods)
    {
        if (eMethodStatic == method->type() && args.size() == method->numArguments() && methodName.equals(method->name()))
        {
            try
            {
                return method->invoker()->invoke(this, args);
            }
            catch (const BindArgumentException& /*ex*/)
            {
                continue;
            }
        }
    }
    throw MethodNotFoundException(String("Cannot find static method " + methodName + " compatible with arguments provided").c_str());
}

void MetaObject::prepare() const
{
    // double checked lock
    if (m_initialized) return;
    {
        std::lock_guard<std::mutex> _guard(m_mutex);
        if (m_initialized) return;
        MetaFieldFactory fieldFactory;
        MetaCallFactory methodFactory;
        for (const MetaInfoDescriptor *desc = m_descriptor; desc; desc = desc->m_superDescriptor)
        {
            if (desc->m_fieldDescriptors)
            {
                for (size_t i = 0; desc->m_fieldDescriptors[i].m_pszName; ++i)
                {
                    m_fields.push_back(std::move(fieldFactory.createInstance(desc->m_fieldDescriptors + i, this)));
                }
            }
            if (desc->m_methodDescriptors)
            {
                for (size_t i = 0; desc->m_methodDescriptors[i].m_pszName; ++i)
                {
                    m_methods.push_back(std::move(methodFactory.createInstance(desc->m_methodDescriptors + i)));
                }
            }
        }
        std::sort(m_fields.begin(), m_fields.end(), [](const std::unique_ptr<MetaFieldBase>& a, const std::unique_ptr<MetaFieldBase>& b)
            {return a->offset() < b->offset(); });
        m_initialized.store(true);
    }
}


MetaFieldBase::MetaFieldBase(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : m_descriptor(fieldDescriptor), m_metaObject(metaObject)
{

}

MetaFieldBase::~MetaFieldBase()
{
}

const char *MetaFieldBase::name() const
{
    return m_descriptor->m_pszName;
}

size_t MetaFieldBase::size() const
{
    return m_descriptor->m_dwSize;
}

ptrdiff_t MetaFieldBase::offset() const
{
    return m_descriptor->m_dwOffset;
}

EFieldType MetaFieldBase::type() const
{
    return m_descriptor->m_eType;
}

bool MetaFieldBase::nullable() const
{
    return m_descriptor->m_nullable;
}

EMandatoriness MetaFieldBase::mandatoriness() const
{
    return m_descriptor->valueInfo.mandatoriness;
}

const MetaObject *MetaFieldBase::metaObject() const
{
    return m_metaObject;
}

MetaFieldBool::MetaFieldBool(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : MetaField<bool>(fieldDescriptor, metaObject)
{
}

bool MetaFieldBool::defaultValue() const
{
    return m_descriptor->valueInfo.ext.m_bool.defaultValue;
}

MetaFieldInt::MetaFieldInt(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : MetaField<int32_t>(fieldDescriptor, metaObject)
{
}

int32_t MetaFieldInt::defaultValue() const
{
    return m_descriptor->valueInfo.ext.m_int.defaultValue;
}

MetaFieldUint::MetaFieldUint(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : MetaField<uint32_t>(fieldDescriptor, metaObject)
{
}

uint32_t MetaFieldUint::defaultValue() const
{
    return m_descriptor->valueInfo.ext.m_uint.defaultValue;
}

MetaFieldInt64::MetaFieldInt64(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : MetaField<int64_t>(fieldDescriptor, metaObject)
{
}

int64_t MetaFieldInt64::defaultValue() const
{
    return m_descriptor->valueInfo.ext.m_int64.defaultValue;
}

MetaFieldUint64::MetaFieldUint64(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : MetaField<uint64_t>(fieldDescriptor, metaObject)
{
}

uint64_t MetaFieldUint64::defaultValue() const
{
    return m_descriptor->valueInfo.ext.m_uint64.defaultValue;
}

MetaFieldFloat::MetaFieldFloat(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : MetaField<float>(fieldDescriptor, metaObject)
{
}

float MetaFieldFloat::defaultValue() const
{
    return m_descriptor->valueInfo.ext.m_float.defaultValue;
}

MetaFieldDouble::MetaFieldDouble(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : MetaField<double>(fieldDescriptor, metaObject)
{
}

double MetaFieldDouble::defaultValue() const
{
    return m_descriptor->valueInfo.ext.m_double.defaultValue;
}

MetaFieldString::MetaFieldString(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : MetaField<String>(fieldDescriptor, metaObject)
{
}

const char *MetaFieldString::defaultValue() const
{
    return m_descriptor->valueInfo.ext.m_string.defaultValue;
}

MetaFieldEnum::MetaFieldEnum(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : MetaField<uint32_t>(fieldDescriptor, metaObject)
{
}

uint32_t MetaFieldEnum::defaultValue() const
{
    return m_descriptor->valueInfo.ext.m_enum.enumInfo->m_defaultValue;
}

EEnumType MetaFieldEnum::enumType() const
{
    return m_descriptor->valueInfo.ext.m_enum.enumInfo->m_type;
}

const char *MetaFieldEnum::enumName() const
{
    return m_descriptor->valueInfo.ext.m_enum.enumInfo->m_enumName;
}

const char *MetaFieldEnum::toString(uint32_t value) const
{
    for (const EnumValueInfoDescriptor *desc = m_descriptor->valueInfo.ext.m_enum.enumInfo->m_valueDescriptors; desc->m_pszValue; ++desc)
        if (desc->m_uValue == value) return desc->m_pszValue;
    return nullptr;
}

uint32_t MetaFieldEnum::fromString(const char *strValue) const
{
    String s(strValue);
    for (const EnumValueInfoDescriptor *desc = m_descriptor->valueInfo.ext.m_enum.enumInfo->m_valueDescriptors; desc->m_pszValue; ++desc)
        if (s == desc->m_pszValue) return desc->m_uValue;
    return defaultValue();
}

MetaFieldObject::MetaFieldObject(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : MetaFieldBase(fieldDescriptor, metaObject)
{
}

Variant MetaFieldObject::getValue(const Object *) const
{
    throw std::runtime_error("MetaFieldObject::getValue() not implemented");
}

void MetaFieldObject::setValue(const Variant &, Object *) const
{
    throw std::runtime_error("MetaFieldObject::setValue() not implemented");
}

const MetaObject *MetaFieldObject::fieldMetaObject() const
{
    return m_descriptor->valueInfo.ext.m_obj.metaObject;
}

MetaFieldArray::MetaFieldArray(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : MetaFieldBase(fieldDescriptor, metaObject)
{
}

Variant MetaFieldArray::getValue(const Object *) const
{
    throw std::runtime_error("MetaFieldArray::getValue() not implemented");
}

void MetaFieldArray::setValue(const Variant &, Object *) const
{
    throw std::runtime_error("MetaFieldArray::setValue() not implemented");
}

EFieldType MetaFieldArray::arrayElementType() const
{
    return m_descriptor->valueInfo.ext.m_array.elemType;
}

size_t MetaFieldArray::arrayElementSize() const
{
    return m_descriptor->valueInfo.ext.m_array.elemSize;
}

MetaFieldDateTime::MetaFieldDateTime(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
    : MetaField<DateTime>(fieldDescriptor, metaObject)
{
}

DateTime MetaFieldDateTime::defaultValue() const
{
    return DateTime(m_descriptor->valueInfo.ext.m_datetime.defaultValue);
}

std::unique_ptr<MetaFieldBase> MetaFieldFactory::createInstance(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
{
    std::unique_ptr<MetaFieldBase> result;
    switch (fieldDescriptor->m_eType)
    {
    case eFieldBool:
        result.reset(new MetaFieldBool(fieldDescriptor, metaObject));
        break;
    case eFieldInt:
        result.reset(new MetaFieldInt(fieldDescriptor, metaObject));
        break;
    case eFieldUint:
        result.reset(new MetaFieldUint(fieldDescriptor, metaObject));
        break;
    case eFieldInt64:
        result.reset(new MetaFieldInt64(fieldDescriptor, metaObject));
        break;
    case eFieldUint64:
        result.reset(new MetaFieldUint64(fieldDescriptor, metaObject));
        break;
    case eFieldFloat:
        result.reset(new MetaFieldFloat(fieldDescriptor, metaObject));
        break;
    case eFieldDouble:
        result.reset(new MetaFieldDouble(fieldDescriptor, metaObject));
        break;
    case eFieldString:
        result.reset(new MetaFieldString(fieldDescriptor, metaObject));
        break;
    case eFieldEnum:
        result.reset(new MetaFieldEnum(fieldDescriptor, metaObject));
        break;
    case eFieldObject:
        result.reset(new MetaFieldObject(fieldDescriptor, metaObject));
        break;
    case eFieldArray:
        result.reset(new MetaFieldArray(fieldDescriptor, metaObject));
        break;
    case eFieldDateTime:
        result.reset(new MetaFieldDateTime(fieldDescriptor, metaObject));
        break;
    default:
        break;
    }
    return result;
}

MetaCallBase::MetaCallBase(const MethodInfoDescriptor *descriptor)
    : m_descriptor(descriptor)
{

}

MetaCallBase::~MetaCallBase()
{

}

const char *MetaCallBase::name() const
{
    return m_descriptor->m_pszName;
}

EMethodType MetaCallBase::type() const
{
    return m_descriptor->m_eType;
}

bool MetaCallBase::constness() const
{
    return m_descriptor->m_bConstness;
}

size_t MetaCallBase::numArguments() const
{
    return m_descriptor->m_nArgs;
}

MetaInvokerBase *MetaCallBase::invoker() const
{
    return m_descriptor->m_pInvoker.get();
}

MetaCallOwn::MetaCallOwn(const MethodInfoDescriptor *descriptor)
    : MetaCallBase(descriptor)
{
}

MetaCallStatic::MetaCallStatic(const MethodInfoDescriptor *descriptor)
    : MetaCallBase(descriptor)
{
}

std::unique_ptr<MetaCallBase> MetaCallFactory::createInstance(const MethodInfoDescriptor *descriptor)
{
    std::unique_ptr<MetaCallBase> result;
    switch (descriptor->m_eType)
    {
    case eMethodOwn:
        result.reset(new MetaCallOwn(descriptor));
        break;
    case eMethodStatic:
        result.reset(new MetaCallStatic(descriptor));
        break;
    default:
        break;
    }
    return result;
}

} // namespace metacpp
