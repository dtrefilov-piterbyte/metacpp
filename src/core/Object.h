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
#ifndef OBJECT_H
#define OBJECT_H
#include "config.h"
#include "MetaObject.h"
#include <map>

namespace metacpp
{

namespace detail
{
class VariantData;
}

/** \brief Base class for objects supporting property and method reflection via MetaObject.
 *
 * All other object should derive from this class, must be default constructible and have
 * metainformation descriptor, generated using special macros.
 *
 * TODO: need detailed usage example here
*/
class Object
{
public:
    Object();
    Object(const Object& other);

	virtual ~Object();

    /**
      \brief Destroys object previously created using MetaObject::createInstance
      */
    void deleteThis();

	/**
        \brief initializes object with default values from metainfo
		\throws std::invalid_argument
	*/
    void init();

#ifdef HAVE_JSONCPP
    /**
        \brief Performs json object serialization
		\throws std::invalid_argument
	*/
    String toJson(bool prettyFormatted = true) const;

	/**
        \brief Performs json object deserialization
		\throws std::invalid_argument
	*/
    void fromJson(const String &s, const Array<const MetaObject *>& knownTypes = Array<const MetaObject *>());
#endif

#ifdef HAVE_MONGODB
    /**
        \brief Performs bson object serialization
        \throws std::invalid_argument
    */
    ByteArray toBson() const;

    /**
        \brief Performs bson object deserialization
        \throws std::invalid_argument
    */
    void fromBson(const void *data);
#endif

    /** \brief Calls reflection own method on this object
     * \param methodName - case-sensetive name of the method
     * \param args - array of arguments to pass to the calling function
    */
    Variant invoke(const String& methodName, const VariantArray& args);

    /** \brief Calls reflection const own method on this object
     * \param methodName - case-sensetive name of the method
     * \param args - array of arguments to pass to the calling function
    */
    Variant invoke(const String& methodName, const VariantArray& args) const;

    /** \brief Calls reflection const own method on this object
     * \param methodName - case-sensetive name of the method
     * \param args - array of arguments to pass to the calling function
    */
    template<typename TRet, typename... TArgs>
    TRet invoke(const String& methodName, TArgs... args)
    {
        return variant_cast<TRet>(invoke(methodName, { args... }));
    }

    /** \brief Calls reflection const own method on this object
     * \param methodName - case-sensetive name of the method
     * \param args - array of arguments to pass to the calling function
    */
    template<typename TRet, typename... TArgs>
    TRet invoke(const String& methodName, TArgs... args) const
    {
        return variant_cast<TRet>(invoke(methodName, { args... }));
    }

    /** \brief Sets a static or dynamic field of the object */
    void setProperty(const String& propName, const Variant& val);
    /** \brief Gets object's static or dynamic field */
    Variant getProperty(const String& propName) const;
    /** \brief Returns the MetaObject instance for the object */
    virtual const MetaObject *metaObject() const = 0;
    /** \brief Returns the MetaObject instance for the class */
    static const MetaObject *staticMetaObject();
protected:
    Object& operator=(const Object& rhs);
private:
    Variant doInvoke(const String& methodName, const VariantArray& args, bool constness) const;
private:
    std::map<String, Variant> m_dynamicProperties;
    static const MetaObject ms_metaObject;
    friend class MetaObject;
    friend class detail::VariantData;
};

/** \brief Macro which must be presented in declaration of classes derived from metacpp::Object
 * \relates metacpp::Object
 */
#define META_INFO_DECLARE(ObjName) \
    public: \
        const ::metacpp::MetaObject *metaObject() const override; \
        static const ::metacpp::MetaObject *staticMetaObject(); \
    private: \
        static const ::metacpp::MetaObject ms_metaObject;

/** \brief Macro which must be presented in definition of classes derived from metacpp::Object
 * \relates metacpp::Object
 */
#define META_INFO(ObjName) \
    const ::metacpp::MetaObject ObjName::ms_metaObject(&REFLECTIBLE_DESCRIPTOR(ObjName)); \
    const ::metacpp::MetaObject *ObjName::metaObject() const { return &ms_metaObject; } \
    const ::metacpp::MetaObject *ObjName::staticMetaObject() { return &ms_metaObject; }

/** \brief Get offset of the field into the struct */
template<typename TObj, typename TField>
static constexpr ptrdiff_t getMemberOffset(const TField TObj::*member)
{
    return reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<const TObj *>(NULL)->*member));
}

/** \brief Get a correspondig MetaFieldBase for the specified public field.
 * Usage: getMetaField(&Object::field); */
template<typename TObj, typename TField>
static constexpr const MetaFieldBase *getMetaField(const TField TObj::*member)
{
    return TObj::staticMetaObject()->fieldByOffset(getMemberOffset(member));
}

REFLECTIBLE_DESCRIPTOR_DECLARE(Object)

} // namespace metacpp
#endif // OBJECT_H
