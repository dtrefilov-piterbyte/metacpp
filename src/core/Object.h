#pragma once
#include "MetaObject.h"

namespace orm
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
		\a omit - whether to skip optional fields (ie with mandatoriness eOptional) or not
		\throws std::invalid_argument
	*/
	void init(bool omit = false);
	
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

	virtual const pkMetaObject *metaObject() const = 0;
};

} // namespace pkapi

#define PKMETA_INFO_DECLARE(structName) \
		const pkMetaObject *metaObject() const override { return &ms_metaObject; } \
	private: \
		static pkMetaObject ms_metaObject;

#define PKMETA_INFO(structName) \
	pkMetaObject structName::ms_metaObject(&STRUCT_INFO(structName));
