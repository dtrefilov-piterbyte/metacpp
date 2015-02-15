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
#ifndef METAOBJECT_H
#define METAOBJECT_H
#include "MetaInfo.h"
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include "Utils.h"
#include "Variant.h"

namespace metacpp
{

class MetaFieldBase;
class MetaCallBase;

/**
  * \brief Class represents meta-information about objects
  * \see Object
*/
class MetaObject
{
public:
    explicit MetaObject(const MetaInfoDescriptor *descriptor,
                        Object *(*constructor)(void *mem) = nullptr,
                        void (*destructor)(void *mem) = nullptr);
    ~MetaObject(void);

    const char *name() const;
    const MetaFieldBase *field(size_t i) const;
    const MetaFieldBase *fieldByOffset(ptrdiff_t offset) const;
    const MetaFieldBase *fieldByName(const String& name, bool caseSensetive = true) const;
    size_t totalFields() const;

    const MetaCallBase *method(size_t i) const;
    const MetaCallBase *methodByName(const String& name, bool caseSensetive = true) const;
    size_t totalMethods() const;

    const MetaObject *superMetaObject() const;
    size_t size() const;

    Object *createInstance() const;
    void destroyInstance(Object *object) const;

    Variant invoke(const String& methodName, const VariantArray& args) const;

    template<typename TRet, typename... TArgs>
    TRet invoke(const String &methodName, TArgs... args) const
    {
        return variant_cast<TRet>(invoke(methodName, { args... }));
    }
private:
    void prepare() const;

    const MetaInfoDescriptor *m_descriptor;
    mutable std::atomic<bool> m_initialized;
    mutable std::vector<std::unique_ptr<MetaFieldBase> > m_fields;
    mutable std::vector<std::unique_ptr<MetaCallBase> > m_methods;
    mutable std::unique_ptr<const MetaObject> m_super;
    mutable std::mutex m_mutex;
    Object *(*m_constructor)(void *mem);
    void (*m_destructor)(void *mem);
};

class MetaFieldBase
{
protected:
    explicit MetaFieldBase(const FieldInfoDescriptor *fieldDescriptor);
public:
    virtual ~MetaFieldBase();

    MetaFieldBase(const MetaFieldBase&)=delete;
    MetaFieldBase& operator=(const MetaFieldBase& rhs)=delete;

    virtual const char *name() const;
    virtual size_t size() const;
    virtual ptrdiff_t offset() const;
    virtual EFieldType type() const;
    virtual bool nullable() const;
    virtual EMandatoriness mandatoriness() const;
    virtual Variant getValue(const Object *obj) const = 0;
    virtual void setValue(const Variant& val, Object *obj) const = 0;

    template<typename T>
    T& access(Object *obj) const {
        return *reinterpret_cast<T *>(reinterpret_cast<char *>(obj) + offset());
    }

    template<typename T>
    const T& access(const Object *obj) const {
        return *reinterpret_cast<const T *>(reinterpret_cast<const char *>(obj) + offset());
    }
protected:
    const FieldInfoDescriptor *m_descriptor;
};

template<typename T>
class MetaField : public MetaFieldBase
{
protected:
    explicit MetaField(const FieldInfoDescriptor *fieldDescriptor)
        : MetaFieldBase(fieldDescriptor)
    {
    }

    Variant getValue(const Object *obj) const override
    {
        if (nullable())
        {
            auto& f = access<Nullable<T>>(obj);
            return f ? Variant(*f) : Variant();
        }
        else
            return access<T>(obj);
    }

    void setValue(const Variant& val, Object *obj) const override
    {
        if (nullable())
        {
            if (val.valid())
                access<Nullable<T> >(obj) = variant_cast<T>(val);
            else
                access<Nullable<T> >(obj).reset();
        }
        else
            access<T>(obj) = variant_cast<T>(val);
    }
};

class MetaFieldBool : public MetaField<bool>
{
public:
    MetaFieldBool(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline bool defaultValue() const { return m_descriptor->valueInfo.ext.m_bool.defaultValue; }
};

class MetaFieldInt : public MetaField<int32_t>
{
public:
    MetaFieldInt(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline int32_t defaultValue() const { return m_descriptor->valueInfo.ext.m_int.defaultValue; }
};

class MetaFieldUint : public MetaField<uint32_t>
{
public:
    MetaFieldUint(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline uint32_t defaultValue() const { return m_descriptor->valueInfo.ext.m_uint.defaultValue; }
};

class MetaFieldInt64 : public MetaField<int64_t>
{
public:
    MetaFieldInt64(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline int64_t defaultValue() const { return m_descriptor->valueInfo.ext.m_int64.defaultValue; }
};

class MetaFieldUint64 : public MetaField<uint64_t>
{
public:
    MetaFieldUint64(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline uint64_t defaultValue() const { return m_descriptor->valueInfo.ext.m_uint64.defaultValue; }
};

class MetaFieldFloat : public MetaField<float>
{
public:
    MetaFieldFloat(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline float defaultValue() const { return m_descriptor->valueInfo.ext.m_float.defaultValue; }
};

class MetaFieldDouble : public MetaField<double>
{
public:
    MetaFieldDouble(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline double defaultValue() const { return m_descriptor->valueInfo.ext.m_double.defaultValue; }
};

class MetaFieldString : public MetaField<String>
{
public:
    MetaFieldString(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline const char *defaultValue() const { return m_descriptor->valueInfo.ext.m_string.defaultValue; }
};

class MetaFieldEnum : public MetaField<uint32_t>
{
public:
    MetaFieldEnum(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline uint32_t defaultValue() const { return m_descriptor->valueInfo.ext.m_enum.enumInfo->m_defaultValue; }
    inline EEnumType enumType() const { return m_descriptor->valueInfo.ext.m_enum.enumInfo->m_type; }
    inline const char *enumName() const { return m_descriptor->valueInfo.ext.m_enum.enumInfo->m_enumName; }

    inline const char *toString(uint32_t value) const
    {
        for (const EnumValueInfoDescriptor *desc = m_descriptor->valueInfo.ext.m_enum.enumInfo->m_valueDescriptors; desc->m_pszValue; ++desc)
            if (desc->m_uValue == value) return desc->m_pszValue;
        return nullptr;
    }

    inline uint32_t fromString(const char *strValue) const
    {
        String s(strValue);
        for (const EnumValueInfoDescriptor *desc = m_descriptor->valueInfo.ext.m_enum.enumInfo->m_valueDescriptors; desc->m_pszValue; ++desc)
            if (s == desc->m_pszValue) return desc->m_uValue;
        return defaultValue();
    }
};

class MetaFieldObject : public MetaFieldBase
{
public:
    MetaFieldObject(const FieldInfoDescriptor *fieldDescriptor)
        : MetaFieldBase(fieldDescriptor)
    {
    }

    Variant getValue(const Object */*obj*/) const override
    {
        throw std::runtime_error("MetaFieldObject::getValue() not implemented");
    }

    void setValue(const Variant& /*val*/, Object */*obj*/) const override
    {
        throw std::runtime_error("MetaFieldObject::setValue() not implemented");
    }

    const MetaObject *metaObject() const { return m_descriptor->valueInfo.ext.m_obj.metaObject; }
};

class MetaFieldArray : public MetaFieldBase
{
public:
    MetaFieldArray(const FieldInfoDescriptor *fieldDescriptor)
        : MetaFieldBase(fieldDescriptor)
    {
    }

    Variant getValue(const Object */*obj*/) const override
    {
        throw std::runtime_error("MetaFieldArray::getValue() not implemented");
    }

    void setValue(const Variant& /*val*/, Object */*obj*/) const override
    {
        throw std::runtime_error("MetaFieldArray::setValue() not implemented");
    }

    inline EFieldType arrayElementType() const { return m_descriptor->valueInfo.ext.m_array.elemType; }
    inline size_t arrayElementSize() const { return m_descriptor->valueInfo.ext.m_array.elemSize; }
};

class MetaFieldDateTime : public MetaField<DateTime>
{
public:
    MetaFieldDateTime(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline DateTime defaultValue() const { return DateTime(m_descriptor->valueInfo.ext.m_datetime.defaultValue); }
};

class MetaFieldFactory : public FactoryBase<std::unique_ptr<MetaFieldBase>, const FieldInfoDescriptor *>
{
public:
    std::unique_ptr<MetaFieldBase> createInstance(const FieldInfoDescriptor *arg) override;
};


class MetaCallBase
{
public:
    explicit MetaCallBase(const MethodInfoDescriptor *descriptor);
    virtual ~MetaCallBase();

    const char *name() const;
    EMethodType type() const;
    bool constness() const;
    size_t numArguments() const;
    MetaInvokerBase *invoker() const;
protected:
    const MethodInfoDescriptor *m_descriptor;
};

class MetaCallOwn : public MetaCallBase
{
public:
    explicit MetaCallOwn(const MethodInfoDescriptor *descriptor);

};

class MetaCallStatic : public MetaCallBase
{
public:
    explicit MetaCallStatic(const MethodInfoDescriptor *descriptor);

};

class MetaCallFactory : public FactoryBase<std::unique_ptr<MetaCallBase>, const MethodInfoDescriptor *>
{
public:
    std::unique_ptr<MetaCallBase> createInstance(const MethodInfoDescriptor *arg) override;
};

} // namespace metacpp
#endif // METAOBJECT_H
