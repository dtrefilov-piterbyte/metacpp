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
    explicit MetaObject(const MetaInfoDescriptor *descriptor);
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

    /** \brief Creates a new instance of the object of corresponding class
     *  with given arguments passed to it's constructor via VariantArray */
    Object *createInstance(const VariantArray& args) const;

    /** \brief Creates a new instance of the object of corresponding class
     * with specified arguments passed to it's constructor
    */
    template<typename... TArgs>
    Object *createInstance(TArgs... args) const {
        return createInstance({ args... });
    }

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
};

/** \brief Abstract base class representing property reflection info */
class MetaFieldBase
{
protected:
    /** \brief Constructs new instance of MetaFieldBase with given fieldDescriptor */
    explicit MetaFieldBase(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);
public:
    virtual ~MetaFieldBase();

    /** \brief Deleted copy constructor */
    MetaFieldBase(const MetaFieldBase&)=delete;
    /** \brief Deleted assignment operator */
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
    /** \brief Checks whether this field is of integral type */
    virtual bool isIntegral() const { return false; }
    /** \brief Checks whether this is field is of floating point type */
    virtual bool isFloatingPoint() const { return false; }

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

    const MetaObject *metaObject() const;
protected:
    const FieldInfoDescriptor *m_descriptor;
    const MetaObject *m_metaObject;
};

/** \brief Base clas for property reflection info of scalar types */
template<typename T>
class MetaField : public MetaFieldBase
{
protected:
    /** \brief Constructs new instance of MetaField using given fieldDescriptor */
    explicit MetaField(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject)
        : MetaFieldBase(fieldDescriptor, metaObject)
    {
    }

    /** \brief Gets of value of the field represented as metacpp::Variant */
    Variant getValue(const Object *obj) const override
    {
        if (nullable())
        {
            auto& f = access<Nullable<T>>(obj);
            return f.isSet() ? Variant(*f) : Variant();
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

    /** \brief Overriden from MetaFieldBase::isIntegral */
    bool isIntegral() const override { return std::is_integral<T>::value; }
    /** \brief Overriden from MetaFieldBase::isFloatingPoint */
    bool isFloatingPoint() const override { return std::is_floating_point<T>::value; }
};

/** \brief Represents bool property reflection info */
class MetaFieldBool : public MetaField<bool>
{
public:
    /** \brief Constructs new instance of MetaFieldBool using given fieldDescriptor */
    MetaFieldBool(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);
    /** \brief Gets the default value of this field */
    bool defaultValue() const;
};

/** \brief Represents int32_t property reflection info */
class MetaFieldInt : public MetaField<int32_t>
{
public:
    /** \brief Constructs new instance of MetaFieldInt using given fieldDescriptor */
    MetaFieldInt(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);
    /** \brief Gets the default value of this field */
    int32_t defaultValue() const;
};

/** \brief Represents uint32_t property reflection info */
class MetaFieldUint : public MetaField<uint32_t>
{
public:
    /** \brief Constructs new instance of MetaFieldUint using given fieldDescriptor */
    MetaFieldUint(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);
    /** \brief Gets the default value of this field */
    uint32_t defaultValue() const;
};

/** \brief Represents int64_t property reflection info */
class MetaFieldInt64 : public MetaField<int64_t>
{
public:
    /** \brief Constructs new instance of MetaFieldInt64 using given fieldDescriptor */
    MetaFieldInt64(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);
    /** \brief Gets the default value of this field */
    int64_t defaultValue() const;
};

/** \brief Represents uint64_t property reflection info */
class MetaFieldUint64 : public MetaField<uint64_t>
{
public:
    /** \brief Constructs new instance of MetaFieldUint64 using given fieldDescriptor */
    MetaFieldUint64(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);
    /** \brief Gets the default value of this field */
    uint64_t defaultValue() const;
};

/** \brief Represents float property reflection info */
class MetaFieldFloat : public MetaField<float>
{
public:
    /** \brief Constructs new instance of MetaFieldFloat using given fieldDescriptor */
    MetaFieldFloat(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);
    /** \brief Gets the default value of this field */
    float defaultValue() const;
};

/** \brief Represents double property reflection info */
class MetaFieldDouble : public MetaField<double>
{
public:
    /** \brief Constructs new instance of MetaFieldDouble using given fieldDescriptor */
    MetaFieldDouble(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);

    /** \brief Gets the default value of this field */
    double defaultValue() const;
};

/** \brief Represents metacpp::String property reflection info */
class MetaFieldString : public MetaField<String>
{
public:
    /** \brief Constructs new instance of MetaFieldString using given fieldDescriptor */
    MetaFieldString(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);
    /** \brief Gets the default value of this field */
    const char *defaultValue() const;
};

/** \brief Represents enum property reflection info */
class MetaFieldEnum : public MetaField<uint32_t>
{
public:
    /** \brief Constructs new instance of MetaFieldEnum using given fieldDescriptor */
    MetaFieldEnum(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);
    /** \brief Gets the default value of this field */
    uint32_t defaultValue() const;
    /** \brief Gets the enum type of this field */
    EEnumType enumType() const;
    /** \brief Gets the enumeration name of this field */
    const char *enumName() const;

    /** \brief Trys to resolve uint32_t value of enumeration as it's name
     *
     * \returns nullptr if failed
     */
    const char *toString(uint32_t value) const;

    /** \brief Trys to resolve named value of enumeration as uint32_t
     *
     * \returns nullptr if failed
     */
    uint32_t fromString(const char *strValue) const;
};

/** \brief Represents metacpp::Object property reflection info */
class MetaFieldObject : public MetaFieldBase
{
public:
    /** \brief Constructs new instance of MetaFieldObject using given fieldDescriptor */
    MetaFieldObject(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);
    /** \brief Overriden from MetaFieldBase::getValue, throws std::runtime_error */
    Variant getValue(const Object *) const override;
    /** \brief Overriden from MetaFieldBase::setValue, throws std::runtime_error */
    void setValue(const Variant&, Object *) const override;
    /** \brief Gets a metacpp::MetaObject for this object field */
    const MetaObject *fieldMetaObject() const;
};

/** \brief Represents metacpp::Array property reflection info */
class MetaFieldArray : public MetaFieldBase
{
public:
    /** \brief Constructs new instance of MetaFieldArray using given fieldDescriptor */
    MetaFieldArray(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);
    /** \brief Overriden from MetaFieldBase::getValue, throws std::runtime_error */
    Variant getValue(const Object *) const override;
    /** \brief Overriden from MetaFieldBase::setValue, throws std::runtime_error */
    void setValue(const Variant&, Object *) const override;
    /** \brief Gets a type of the element of this array field */
    EFieldType arrayElementType() const;
    /** \brief Gets a size of the element of this array field */
    size_t arrayElementSize() const;
};

/** \brief Represents metacpp::DateTime property reflection info */
class MetaFieldDateTime : public MetaField<DateTime>
{
public:
    /** \brief Constructs new instance of MetaFieldDateTime using given fieldDescriptor */
    MetaFieldDateTime(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject);
    /** \brief Gets the default value of this field */
    DateTime defaultValue() const;
};

/** \brief Factory class for the property reflection infos */
class MetaFieldFactory : public FactoryBase<std::unique_ptr<MetaFieldBase>, const FieldInfoDescriptor *, const MetaObject *>
{
public:
    /** \brief Creates an instance of reflection info using given fieldDescriptor */
    std::unique_ptr<MetaFieldBase> createInstance(const FieldInfoDescriptor *fieldDescriptor, const MetaObject *metaObject) override;
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
