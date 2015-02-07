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

namespace metacpp
{

/**
	A basic class for representing objects with metainfo
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
	
	/**
		performs json object serialization
		\throws std::invalid_argument
	*/
    String toString(bool prettyFormatted = true) const;

	/**
		performs json object deserialization
		\throws std::invalid_argument
	*/
    void fromString(const String &s);

    /**
     * Calls own method on this object
     * \param methodName - case-sensetive name of the method
     * \param args - array of arguments to pass to the calling function
    */
    Variant invoke(const String& methodName, const VariantArray& args);
    /**
     * Calls const own method on this object
     * \param methodName - case-sensetive name of the method
     * \param args - array of arguments to pass to the calling function
    */
    Variant invoke(const String& methodName, const VariantArray& args) const;

	virtual const MetaObject *metaObject() const = 0;
private:
    Variant doInvoke(const String& methodName, const VariantArray& args, bool constness) const;
};

#define META_INFO_DECLARE(ObjName) \
        const MetaObject *metaObject() const override; \
        static const MetaObject *staticMetaObject(); \
        static Object *constructInstance(void *mem); \
        static void destructInstance(void *mem); \
    private: \
        static MetaObject ms_metaObject;

#define META_INFO(ObjName) \
    MetaObject ObjName::ms_metaObject(&REFLECTIBLE_DESCRIPTOR(ObjName), \
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

/** \brief Usage: getMetaField(&Object::field); */
template<typename TObj, typename TField>
static constexpr const MetaField *getMetaField(const TField TObj::*member)
{
    return TObj::staticMetaObject()->fieldByOffset(getMemberOffset(member));
}

REFLECTIBLE_DESCRIPTOR_DECLARE(Object)

} // namespace metacpp
#endif // OBJECT_H
