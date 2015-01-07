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

	virtual const MetaObject *metaObject() const = 0;
};

#define META_INFO_DECLARE(structName) \
        const MetaObject *metaObject() const override { return &ms_metaObject; } \
        static const MetaObject *staticMetaObject() { return &ms_metaObject; } \
    private: \
        static MetaObject ms_metaObject;

#define META_INFO(structName) \
    MetaObject structName::ms_metaObject(&STRUCT_INFO(structName));

STRUCT_INFO_DECLARE(Object)


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

} // namespace metacpp
#endif // OBJECT_H
