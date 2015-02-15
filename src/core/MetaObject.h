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
  * \brief Class represents meta-information about classes
  * \see Object
*/
class MetaObject
{
public:
    /** \brief Constructs a new instance of MetaObject with given descriptor, constructor and desctructor callbacks */
    explicit MetaObject(const MetaInfoDescriptor *descriptor,
                        Object *(*constructor)(void *mem) = nullptr,
                        void (*destructor)(void *mem) = nullptr);
    ~MetaObject(void);

    /** \brief Returns a name of the class */
    const char *name() const;
    /** \brief Gets a field reflection info at specified position i */
    const MetaFieldBase *field(size_t i) const;
    /** \brief Gets a field reflection info at specified offset in class */
    const MetaFieldBase *fieldByOffset(ptrdiff_t offset) const;
    /** \brief Gets a field reflection info by it's name */
    const MetaFieldBase *fieldByName(const String& name, bool caseSensetive = true) const;
    /** \brief Gets a total number of field reflection infos */
    size_t totalFields() const;

    /** \brief Gets a method reflection info at the specified position */
    const MetaCallBase *method(size_t i) const;
    /** \brief Gets a method reflection info by it's name */
    const MetaCallBase *methodByName(const String& name, bool caseSensetive = true) const;
    /** \brief Gets a total number of method reflection infos for this class */
    size_t totalMethods() const;

    /** \brief Gets a MetaObject for the super class */
    const MetaObject *superMetaObject() const;
    /** \brief Gets a size of the class  */
    size_t size() const;

    /** \brief Creates a new instance of the object of corresponding class */
    Object *createInstance() const;
    /** \brief Destroys instance of the object previously created with createInstance */
    void destroyInstance(Object *object) const;

    /** \brief Performs metacall to the static named method with specified args */
    Variant invoke(const String& methodName, const VariantArray& args) const;

    /** \brief Performs metacall to the statuc named method with specified args */
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

/** \brief Abstract base class representing property reflection info */
class MetaFieldBase
{
protected:
    /** \brief Constructs new instance of MetaFieldBase with given fieldDescriptor */
    explicit MetaFieldBase(const FieldInfoDescriptor *fieldDescriptor);
public:
    virtual ~MetaFieldBase();

    MetaFieldBase(const MetaFieldBase&)=delete;
    MetaFieldBase& operator=(const MetaFieldBase& rhs)=delete;

    /** \brief Gets a name of the field */
    virtual const char *name() const;
    /** \brief Gets the size of the property */
    virtual size_t size() const;
    /** \brief Gets the offset of the field in Object */
    virtual ptrdiff_t offset() const;
    /** \brief Gets the type of the field */
    virtual EFieldType type() const;
    /** \brief Checks whether this is a Nullable<T> field */
    virtual bool nullable() const;
    /** \brief Gets field mandatoriness */
    virtual EMandatoriness mandatoriness() const;
    /** \brief Gets a value of the field in specified object represented as metacpp::Variant (if possible) */
    virtual Variant getValue(const Object *object) const = 0;
    /** \brief Sets a value of the in specified object using value of metacpp::Variant (if possible) */
    virtual void setValue(const Variant& val, Object *object) const = 0;

    /** \brief Accesses a field in the object using this reflection info */
    template<typename T>
    T& access(Object *obj) const {
        return *reinterpret_cast<T *>(reinterpret_cast<char *>(obj) + offset());
    }

    /** \brief Accesses a field in the object using this reflection info */
    template<typename T>
    const T& access(const Object *obj) const {
        return *reinterpret_cast<const T *>(reinterpret_cast<const char *>(obj) + offset());
    }
protected:
    const FieldInfoDescriptor *m_descriptor;
};

/** \brief Base clas for property reflection info of scalar types */
template<typename T>
class MetaField : public MetaFieldBase
{
protected:
    /** \brief Constructs new instance of MetaField using given fieldDescriptor */
    explicit MetaField(const FieldInfoDescriptor *fieldDescriptor)
        : MetaFieldBase(fieldDescriptor)
    {
    }

    /** \brief Gets of value of the field represented as metacpp::Variant */
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

    /** \brief Sets a value of the in specified object using value of metacpp::Variant */
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

/** \brief Represents bool property reflection info */
class MetaFieldBool : public MetaField<bool>
{
public:
    /** \brief Constructs new instance of MetaFieldBool using given fieldDescriptor */
    MetaFieldBool(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    /** \brief Gets the default value of this field */
    inline bool defaultValue() const { return m_descriptor->valueInfo.ext.m_bool.defaultValue; }
};

/** \brief Represents int32_t property reflection info */
class MetaFieldInt : public MetaField<int32_t>
{
public:
    /** \brief Constructs new instance of MetaFieldInt using given fieldDescriptor */
    MetaFieldInt(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    /** \brief Gets the default value of this field */
    inline int32_t defaultValue() const { return m_descriptor->valueInfo.ext.m_int.defaultValue; }
};

/** \brief Represents uint32_t property reflection info */
class MetaFieldUint : public MetaField<uint32_t>
{
public:
    /** \brief Constructs new instance of MetaFieldUint using given fieldDescriptor */
    MetaFieldUint(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    /** \brief Gets the default value of this field */
    inline uint32_t defaultValue() const { return m_descriptor->valueInfo.ext.m_uint.defaultValue; }
};

/** \brief Represents int64_t property reflection info */
class MetaFieldInt64 : public MetaField<int64_t>
{
public:
    /** \brief Constructs new instance of MetaFieldInt64 using given fieldDescriptor */
    MetaFieldInt64(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    /** \brief Gets the default value of this field */
    inline int64_t defaultValue() const { return m_descriptor->valueInfo.ext.m_int64.defaultValue; }
};

/** \brief Represents uint64_t property reflection info */
class MetaFieldUint64 : public MetaField<uint64_t>
{
public:
    /** \brief Constructs new instance of MetaFieldUint64 using given fieldDescriptor */
    MetaFieldUint64(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    /** \brief Gets the default value of this field */
    inline uint64_t defaultValue() const { return m_descriptor->valueInfo.ext.m_uint64.defaultValue; }
};

/** \brief Represents float property reflection info */
class MetaFieldFloat : public MetaField<float>
{
public:
    /** \brief Constructs new instance of MetaFieldFloat using given fieldDescriptor */
    MetaFieldFloat(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    /** \brief Gets the default value of this field */
    inline float defaultValue() const { return m_descriptor->valueInfo.ext.m_float.defaultValue; }
};

/** \brief Represents double property reflection info */
class MetaFieldDouble : public MetaField<double>
{
public:
    /** \brief Constructs new instance of MetaFieldDouble using given fieldDescriptor */
    MetaFieldDouble(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    /** \brief Gets the default value of this field */
    inline double defaultValue() const { return m_descriptor->valueInfo.ext.m_double.defaultValue; }
};

/** \brief Represents metacpp::String property reflection info */
class MetaFieldString : public MetaField<String>
{
public:
    /** \brief Constructs new instance of MetaFieldString using given fieldDescriptor */
    MetaFieldString(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    /** \brief Gets the default value of this field */
    inline const char *defaultValue() const { return m_descriptor->valueInfo.ext.m_string.defaultValue; }
};

/** \brief Represents enum property reflection info */
class MetaFieldEnum : public MetaField<uint32_t>
{
public:
    /** \brief Constructs new instance of MetaFieldEnum using given fieldDescriptor */
    MetaFieldEnum(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    /** \brief Gets the default value of this field */
    inline uint32_t defaultValue() const { return m_descriptor->valueInfo.ext.m_enum.enumInfo->m_defaultValue; }

    /** \brief Gets the enum type of this field */
    inline EEnumType enumType() const { return m_descriptor->valueInfo.ext.m_enum.enumInfo->m_type; }
    /** \brief Gets the enumeration name of this field */
    inline const char *enumName() const { return m_descriptor->valueInfo.ext.m_enum.enumInfo->m_enumName; }

    /** \brief Trys to resolve uint32_t value of enumeration as it's name
     *
     * \returns nullptr if failed
     */
    inline const char *toString(uint32_t value) const
    {
        for (const EnumValueInfoDescriptor *desc = m_descriptor->valueInfo.ext.m_enum.enumInfo->m_valueDescriptors; desc->m_pszValue; ++desc)
            if (desc->m_uValue == value) return desc->m_pszValue;
        return nullptr;
    }

    /** \brief Trys to resolve named value of enumeration as uint32_t
     *
     * \returns nullptr if failed
     */
    inline uint32_t fromString(const char *strValue) const
    {
        String s(strValue);
        for (const EnumValueInfoDescriptor *desc = m_descriptor->valueInfo.ext.m_enum.enumInfo->m_valueDescriptors; desc->m_pszValue; ++desc)
            if (s == desc->m_pszValue) return desc->m_uValue;
        return defaultValue();
    }
};

/** \brief Represents metacpp::Object property reflection info */
class MetaFieldObject : public MetaFieldBase
{
public:
    /** \brief Constructs new instance of MetaFieldObject using given fieldDescriptor */
    MetaFieldObject(const FieldInfoDescriptor *fieldDescriptor)
        : MetaFieldBase(fieldDescriptor)
    {
    }

    /** \brief Overriden from MetaFieldBase::getValue, throws std::runtime_error */
    Variant getValue(const Object */*obj*/) const override
    {
        throw std::runtime_error("MetaFieldObject::getValue() not implemented");
    }

    /** \brief Overriden from MetaFieldBase::setValue, throws std::runtime_error */
    void setValue(const Variant& /*val*/, Object */*obj*/) const override
    {
        throw std::runtime_error("MetaFieldObject::setValue() not implemented");
    }

    /** \brief Gets a metacpp::MetaObject for this object field */
    const MetaObject *metaObject() const { return m_descriptor->valueInfo.ext.m_obj.metaObject; }
};

/** \brief Represents metacpp::Array property reflection info */
class MetaFieldArray : public MetaFieldBase
{
public:
    /** \brief Constructs new instance of MetaFieldArray using given fieldDescriptor */
    MetaFieldArray(const FieldInfoDescriptor *fieldDescriptor)
        : MetaFieldBase(fieldDescriptor)
    {
    }

    /** \brief Overriden from MetaFieldBase::getValue, throws std::runtime_error */
    Variant getValue(const Object */*obj*/) const override
    {
        throw std::runtime_error("MetaFieldArray::getValue() not implemented");
    }

    /** \brief Overriden from MetaFieldBase::setValue, throws std::runtime_error */
    void setValue(const Variant& /*val*/, Object */*obj*/) const override
    {
        throw std::runtime_error("MetaFieldArray::setValue() not implemented");
    }

    /** \brief Gets a type of the element of this array field */
    inline EFieldType arrayElementType() const { return m_descriptor->valueInfo.ext.m_array.elemType; }
    /** \brief Gets a size of the element of this array field */
    inline size_t arrayElementSize() const { return m_descriptor->valueInfo.ext.m_array.elemSize; }
};

/** \brief Represents metacpp::DateTime property reflection info */
class MetaFieldDateTime : public MetaField<DateTime>
{
public:
    /** \brief Constructs new instance of MetaFieldDateTime using given fieldDescriptor */
    MetaFieldDateTime(const FieldInfoDescriptor *fieldDescriptor)
        : MetaField(fieldDescriptor)
    {
    }

    /** \brief Gets the default value of this field */
    inline DateTime defaultValue() const { return DateTime(m_descriptor->valueInfo.ext.m_datetime.defaultValue); }
};

/** \brief Factory class for the property reflection infos */
class MetaFieldFactory : public FactoryBase<std::unique_ptr<MetaFieldBase>, const FieldInfoDescriptor *>
{
public:
    /** \brief Creates an instance of reflection info using given fieldDescriptor */
    std::unique_ptr<MetaFieldBase> createInstance(const FieldInfoDescriptor *fieldDescriptor) override;
};

/** \brief Base abstract class representing method reflection info */
class MetaCallBase
{
public:
    /** \brief Constructs a new instance of MetaCallBase using given descriptor */
    explicit MetaCallBase(const MethodInfoDescriptor *descriptor);
    virtual ~MetaCallBase();

    /** \brief Gets name of the method */
    const char *name() const;
    /** \brief Gets type of the method */
    EMethodType type() const;
    /** \brief Gets constness of self reference of the method */
    bool constness() const;
    /** \brief Gets number of arguments required for the method call */
    size_t numArguments() const;
    /** \brief Gets an invoker for this method */
    MetaInvokerBase *invoker() const;
protected:
    const MethodInfoDescriptor *m_descriptor;
};

/** \brief Represents own method reflection info */
class MetaCallOwn : public MetaCallBase
{
public:
    /** \brief Constructs a new instance of MetaCallOwn using given descriptor */
    explicit MetaCallOwn(const MethodInfoDescriptor *descriptor);

};

/** \brief Represents static method reflection info */
class MetaCallStatic : public MetaCallBase
{
public:
    /** \brief Constructs a new instance of MetaCallStatic using given descriptor */
    explicit MetaCallStatic(const MethodInfoDescriptor *descriptor);

};

/** \brief Factory class for MetaCallBase */
class MetaCallFactory : public FactoryBase<std::unique_ptr<MetaCallBase>, const MethodInfoDescriptor *>
{
public:
    /** \brief Creates MetaCallBase using specified descriptor */
    std::unique_ptr<MetaCallBase> createInstance(const MethodInfoDescriptor *descriptor) override;
};

} // namespace metacpp
#endif // METAOBJECT_H
