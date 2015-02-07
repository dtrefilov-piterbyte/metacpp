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
        throw std::runtime_error("Have no appropriate constructor");
    void *pMem = ::operator new(size());
    return m_constructor(pMem);
}

void MetaObject::destroyInstance(Object *object) const
{
    if (!m_destructor)
        throw std::runtime_error("Have no appropriate constructor");
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
                    m_fields.push_back(std::move(fieldFactory.createInstance(desc->m_fieldDescriptors + i)));
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


MetaFieldBase::MetaFieldBase(const FieldInfoDescriptor *fieldDescriptor)
    : m_descriptor(fieldDescriptor)
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

std::unique_ptr<MetaFieldBase> MetaFieldFactory::createInstance(const FieldInfoDescriptor *arg)
{
    std::unique_ptr<MetaFieldBase> result;
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
    case eFieldInt64:
        result.reset(new MetaFieldInt64(arg));
        break;
    case eFieldUint64:
        result.reset(new MetaFieldUint64(arg));
        break;
    case eFieldFloat:
        result.reset(new MetaFieldFloat(arg));
        break;
    case eFieldDouble:
        result.reset(new MetaFieldDouble(arg));
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
    case eFieldDateTime:
        result.reset(new MetaFieldDateTime(arg));
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

std::unique_ptr<MetaCallBase> MetaCallFactory::createInstance(const MethodInfoDescriptor *arg)
{
    std::unique_ptr<MetaCallBase> result;
    switch (arg->m_eType)
    {
    case eMethodOwn:
        result.reset(new MetaCallOwn(arg));
        break;
    case eMethodStatic:
        result.reset(new MetaCallStatic(arg));
        break;
    default:
        break;
    }
    return result;
}

} // namespace metacpp
