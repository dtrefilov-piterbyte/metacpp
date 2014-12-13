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

	virtual const pkMetaObject *metaObject() const = 0;
};

#define META_INFO_DECLARE(structName) \
        const pkMetaObject *metaObject() const override { return &ms_metaObject; } \
    private: \
        static pkMetaObject ms_metaObject;

#define META_INFO(structName) \
    pkMetaObject structName::ms_metaObject(&STRUCT_INFO(structName));

STRUCT_INFO_DECLARE(Object)

} // namespace metacpp
#endif // OBJECT_H
