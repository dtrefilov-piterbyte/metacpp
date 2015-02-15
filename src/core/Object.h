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
#include "MetaObject.h"
#include <map>

namespace metacpp
{

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
	virtual ~Object();

	/**
		initializes object with default values from metainfo
		\throws std::invalid_argument
	*/
    void init();
	

#ifdef HAVE_JSONCPP
    /**
		performs json object serialization
		\throws std::invalid_argument
	*/
    String toJson(bool prettyFormatted = true) const;

	/**
		performs json object deserialization
		\throws std::invalid_argument
	*/
    void fromJson(const String &s);
#endif

#ifdef HAVE_MONGODB
    /**
        performs bson object serialization
        \throws std::invalid_argument
    */
    ByteArray toBson(bool prettyFormatted = true) const;

    /**
        performs bson object deserialization
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

    template<typename TRet, typename... TArgs>
    TRet invoke(const String& methodName, TArgs... args)
    {
        return variant_cast<TRet>(invoke(methodName, { args... }));
    }

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
private:
    Variant doInvoke(const String& methodName, const VariantArray& args, bool constness) const;
private:
    std::map<String, Variant> m_dynamicProperties;
    static const MetaObject ms_metaObject;
};

#define META_INFO_DECLARE(ObjName) \
        const MetaObject *metaObject() const override; \
        static const MetaObject *staticMetaObject(); \
        static Object *constructInstance(void *mem); \
        static void destructInstance(void *mem); \
    private: \
        static const MetaObject ms_metaObject;

#define META_INFO(ObjName) \
    const MetaObject ObjName::ms_metaObject(&REFLECTIBLE_DESCRIPTOR(ObjName), \
        &ObjName::constructInstance, &ObjName::destructInstance); \
    const MetaObject *ObjName::metaObject() const { return &ms_metaObject; } \
    const MetaObject *ObjName::staticMetaObject() { return &ms_metaObject; } \
    Object *ObjName::constructInstance(void *mem) { return new (mem) ObjName(); } \
    void ObjName::destructInstance(void *mem) { reinterpret_cast<ObjName *>(mem)->~ObjName(); }


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
