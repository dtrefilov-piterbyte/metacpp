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

class MetaField;
class MetaCallBase;

class MetaObject
{
	friend class pkVisitorBase;
public:
    explicit MetaObject(const MetaInfoDescriptor *descriptor,
                        Object *(*constructor)(void *mem) = nullptr,
                        void (*destructor)(void *mem) = nullptr);
    ~MetaObject(void);

    const char *name() const;
    const MetaField *field(size_t i) const;
    const MetaField *fieldByOffset(ptrdiff_t offset) const;
    const MetaField *fieldByName(const String& name, bool caseSensetive = true) const;
    size_t totalFields() const;

    const MetaCallBase *method(size_t i) const;
    const MetaCallBase *methodByName(const String& name, bool caseSensetive = true) const;
    size_t totalMethods() const;

    const MetaObject *superMetaObject() const;
    size_t size() const;

    Object *createInstance() const;
    void destroyInstance(Object *object) const;

    Variant invoke(const String& methodName, const VariantArray& args) const;
private:
    void prepare() const;

    const MetaInfoDescriptor *m_descriptor;
    mutable std::atomic<bool> m_initialized;
    mutable std::vector<std::unique_ptr<MetaField> > m_fields;
    mutable std::vector<std::unique_ptr<MetaCallBase> > m_methods;
    mutable std::unique_ptr<const MetaObject> m_super;
    mutable std::mutex m_mutex;
    Object *(*m_constructor)(void *mem);
    void (*m_destructor)(void *mem);
};

class MetaField
{
protected:
    explicit MetaField(const FieldInfoDescriptor *fieldDescriptor);
public:
    virtual ~MetaField();

    MetaField(const MetaField&)=delete;
    MetaField& operator=(const MetaField& rhs)=delete;

    virtual const char *name() const;
    virtual size_t size() const;
    virtual ptrdiff_t offset() const;
    virtual EFieldType type() const;
    virtual bool nullable() const;
    virtual EMandatoriness mandatoriness() const;

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

class MetaFieldBool : public MetaField
{
public:
    MetaFieldBool(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline bool defaultValue() const { return m_descriptor->valueInfo.ext.m_bool.defaultValue; }
};

class MetaFieldInt : public MetaField
{
public:
    MetaFieldInt(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline int32_t defaultValue() const { return m_descriptor->valueInfo.ext.m_int.defaultValue; }
};

class MetaFieldUint : public MetaField
{
public:
    MetaFieldUint(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline uint32_t defaultValue() const { return m_descriptor->valueInfo.ext.m_uint.defaultValue; }
};

class MetaFieldInt64 : public MetaField
{
public:
    MetaFieldInt64(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline int64_t defaultValue() const { return m_descriptor->valueInfo.ext.m_int64.defaultValue; }
};

class MetaFieldUint64 : public MetaField
{
public:
    MetaFieldUint64(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline uint64_t defaultValue() const { return m_descriptor->valueInfo.ext.m_uint64.defaultValue; }
};

class MetaFieldFloat : public MetaField
{
public:
    MetaFieldFloat(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline float defaultValue() const { return m_descriptor->valueInfo.ext.m_float.defaultValue; }
};

class MetaFieldDouble : public MetaField
{
public:
    MetaFieldDouble(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline double defaultValue() const { return m_descriptor->valueInfo.ext.m_double.defaultValue; }
};

class MetaFieldString : public MetaField
{
public:
    MetaFieldString(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline const char *defaultValue() const { return m_descriptor->valueInfo.ext.m_string.defaultValue; }
};

class MetaFieldEnum : public MetaField
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

class MetaFieldObject : public MetaField
{
public:
    MetaFieldObject(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    const MetaObject *metaObject() const { return m_descriptor->valueInfo.ext.m_obj.metaObject; }
};

class MetaFieldArray : public MetaField
{
public:
    MetaFieldArray(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline EFieldType arrayElementType() const { return m_descriptor->valueInfo.ext.m_array.elemType; }
    inline size_t arrayElementSize() const { return m_descriptor->valueInfo.ext.m_array.elemSize; }
};

class MetaFieldDateTime : public MetaField
{
public:
    MetaFieldDateTime(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    inline DateTime defaultValue() const { return DateTime(m_descriptor->valueInfo.ext.m_datetime.defaultValue); }
};

class MetaFieldFactory : public FactoryBase<std::unique_ptr<MetaField>, const FieldInfoDescriptor *>
{
public:
    std::unique_ptr<MetaField> createInstance(const FieldInfoDescriptor *arg) override;
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
