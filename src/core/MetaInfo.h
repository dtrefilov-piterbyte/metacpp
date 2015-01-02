#ifndef METAINFO_H
#define METAINFO_H
// A very trivial meta-system implementation for automation of module configuration parsing

#include <string>
#include <cstddef>
#include <set>
#include <cstdint>
#include <functional>
#include <stack>
#include <mutex>
#include <ctime>
#include "Array.h"
#include "String.h"
#include "Nullable.h"

namespace metacpp
{
class Object;
}

enum EFieldType
{
	eFieldVoid		= 'x',
	eFieldBool		= 'b',
	eFieldInt		= 'i',
	eFieldUint		= 'u',
    eFieldFloat		= 'f',
    eFieldDouble    = 'd',
	eFieldString	= 's',
	eFieldEnum		= 'e',
	eFieldObject	= 'o',
	eFieldArray		= 'a',
    eFieldTime      = 't',
};

/** \brief Parameter determines assigning behaviour of the field */
enum EMandatoriness
{
    eRequired,			/**< an exception is thrown if field value was not excplicitly specified  */
    eOptional,			/**< ignoring omited descriptor */
    eDefaultable		/**< field is assigned to default value */
};

/** \brief Type of an enum meta info */
enum EEnumType
{
	eEnumNone,
	eEnumSimple,
	eEnumBitset
};


struct EnumValueInfoDescriptor
{
	const char		*m_pszValue;
	uint32_t		m_uValue;
};

struct EnumInfoDescriptor
{
	EEnumType		m_type;
	const char		*m_enumName;
	uint32_t		m_defaultValue;
	EnumValueInfoDescriptor *m_valueDescriptors;
};

struct FieldInfoDescriptor
{
	const char *m_pszName;
	size_t		m_dwSize;
	ptrdiff_t	m_dwOffset;
	EFieldType	m_eType;
    bool        m_nullable;

	struct Extension
	{
        union UExtension
        {
            struct
			{
				const char	*defaultValue;
            } m_string;
            struct
			{
				bool		defaultValue;
            } m_bool;
            struct
			{
				int32_t		defaultValue;
            } m_int;
            struct
			{
				uint32_t		defaultValue;
            } m_uint;
            struct
			{
				float		defaultValue;
            } m_float;
            struct
            {
                double defaultValue;
            } m_double;
            struct
			{
				EnumInfoDescriptor *enumInfo;
            } m_enum;
            struct
			{
                EFieldType      elemType;
                size_t          elemSize;
            } m_array;
        } ext;
        EMandatoriness	mandatoriness;

        explicit Extension()
		{
            mandatoriness = eRequired;
        }
        explicit Extension(bool v)
		{
            ext.m_bool.defaultValue = v;
            mandatoriness = eDefaultable;
        }
		explicit Extension(int32_t v)
		{
            ext.m_int.defaultValue = v;
            mandatoriness = eDefaultable;
        }
		explicit Extension(uint32_t v)
		{
            ext.m_uint.defaultValue = v;
            mandatoriness = eDefaultable;
        }
		explicit Extension(float v)
		{
            ext.m_float.defaultValue = v;
            mandatoriness = eDefaultable;
        }
        explicit Extension(double v)
        {
            ext.m_double.defaultValue = v;
            mandatoriness = eDefaultable;
        }

		explicit Extension(const char *v)
		{
            ext.m_string.defaultValue = v;
            mandatoriness = eDefaultable;
        }
        explicit Extension(EMandatoriness m)
        {
            mandatoriness = m;
        }
        explicit Extension(EnumInfoDescriptor *enumInfo)
		{
            ext.m_enum.enumInfo = enumInfo;
            mandatoriness = eDefaultable;
        }
        explicit Extension(EFieldType elemType, size_t elemSize)
		{
            ext.m_array.elemType = elemType;
            ext.m_array.elemSize = elemSize;
            mandatoriness = eDefaultable;
        }
	} valueInfo;
};

struct StructInfoDescriptor
{
	const char				*m_strucName;
	size_t					m_dwSize;
	StructInfoDescriptor	*m_superDescriptor;
	FieldInfoDescriptor		*m_fieldDescriptors;
};

#if defined(_MSC_VER) && !defined(constexpr)
#define constexpr
#endif

template<typename T>
struct PartialFieldInfoHelper;

template<>
struct PartialFieldInfoHelper<bool> {
	static constexpr EFieldType type() { return eFieldBool; }
    static FieldInfoDescriptor::Extension extension(bool v) { return FieldInfoDescriptor::Extension(v); }
    static FieldInfoDescriptor::Extension extension(EMandatoriness m = eRequired) { return FieldInfoDescriptor::Extension(m); }
};

template<>
struct PartialFieldInfoHelper<int32_t> {
	static constexpr EFieldType type() { return eFieldInt; }
    static FieldInfoDescriptor::Extension extension(int32_t v) { return FieldInfoDescriptor::Extension(v); }
    static FieldInfoDescriptor::Extension extension(EMandatoriness m = eRequired) { return FieldInfoDescriptor::Extension(m); }
};

template<>
struct PartialFieldInfoHelper<uint32_t> {
	static constexpr EFieldType type() { return eFieldUint; }
    static FieldInfoDescriptor::Extension extension(uint32_t v) { return FieldInfoDescriptor::Extension(v); }
    static FieldInfoDescriptor::Extension extension(EMandatoriness m = eRequired) { return FieldInfoDescriptor::Extension(m); }
};

template<>
struct PartialFieldInfoHelper<float> {
	static constexpr EFieldType type() { return eFieldFloat; }
    static FieldInfoDescriptor::Extension extension(float v) { return FieldInfoDescriptor::Extension(v); }
    static FieldInfoDescriptor::Extension extension(EMandatoriness m = eRequired) { return FieldInfoDescriptor::Extension(m); }
};

template<>
struct PartialFieldInfoHelper<double> {
    static constexpr EFieldType type() { return eFieldDouble; }
    static FieldInfoDescriptor::Extension extension(double v) { return FieldInfoDescriptor::Extension(v); }
    static FieldInfoDescriptor::Extension extension(EMandatoriness m = eRequired) { return FieldInfoDescriptor::Extension(m); }
};

template<>
struct PartialFieldInfoHelper<metacpp::String> {
	static constexpr EFieldType type() { return eFieldString; }
    static FieldInfoDescriptor::Extension extension(const char *v) { return FieldInfoDescriptor::Extension(v); }
    static FieldInfoDescriptor::Extension extension(EMandatoriness m = eRequired) { return FieldInfoDescriptor::Extension(m); }
};

template<typename T>
struct PartialFieldInfoHelper<metacpp::Array<T> >
{
    static constexpr EFieldType type() { return eFieldArray; }
	static constexpr FieldInfoDescriptor::Extension extension()
	{
        return FieldInfoDescriptor::Extension(PartialFieldInfoHelper<T>::type(), sizeof(T));
	}
};

template<>
struct PartialFieldInfoHelper<std::time_t>
{
    static constexpr EFieldType type() { return eFieldTime; }
    static FieldInfoDescriptor::Extension extension(EMandatoriness m = eRequired) { return FieldInfoDescriptor::Extension(m); }
};

template<typename T, bool IsEnum = std::is_enum<T>::value, bool IsObject = std::is_base_of<metacpp::Object, T>::value>
struct FullFieldInfoHelper;

template<typename T>
struct FullFieldInfoHelper<T, true, false>
{
	static constexpr EFieldType type() { return eFieldEnum; }
	static constexpr FieldInfoDescriptor::Extension extension(EnumInfoDescriptor *enumInfo) { return FieldInfoDescriptor::Extension(enumInfo); }
    static constexpr bool nullable() { return false; }
};


template<typename T>
struct FullFieldInfoHelper<T, false, false> : public PartialFieldInfoHelper<T>
{
    static constexpr bool nullable() { return false; }
};

template<typename T>
struct FullFieldInfoHelper<T, false, true>
{
	static constexpr EFieldType type() { return eFieldObject; }
    static constexpr FieldInfoDescriptor::Extension extension() { return FieldInfoDescriptor::Extension(eDefaultable); }
    static constexpr bool nullable() { return false; }
};

template<typename T>
struct FullFieldInfoHelper<Nullable<T>, false, false> : public FullFieldInfoHelper<T>
{
    static constexpr bool nullable() { return true; }
};

#define STRUCT_INFO_BEGIN(struc) \
	extern FieldInfoDescriptor _fieldInfos_##struc[]; \
	StructInfoDescriptor _strucInfo_##struc = { #struc, sizeof(struc), nullptr, _fieldInfos_##struc }; \
	FieldInfoDescriptor _fieldInfos_##struc[] = {

#define STRUCT_INFO_DERIVED_BEGIN(struc, super) \
	extern FieldInfoDescriptor _fieldInfos_##struc[]; \
	StructInfoDescriptor _strucInfo_##struc = { #struc, sizeof(struc), &STRUCT_INFO(super), _fieldInfos_##struc }; \
	FieldInfoDescriptor _fieldInfos_##struc[] = {

// field info sequence terminator
#define STRUCT_INFO_END(struc) \
        { 0, 0, 0, eFieldVoid, false, FieldInfoDescriptor::Extension() } \
	};

#define STRUCT_INFO(struc) _strucInfo_##struc
#define STRUCT_INFO_DECLARE(struc) extern StructInfoDescriptor _strucInfo_##struc;

#define FIELD_INFO(struc, field, ...) { \
    /* name */      #field, \
    /* size */      sizeof(decltype(struc::field)), \
    /* offset */    offsetof(struc, field), \
    /* type */      FullFieldInfoHelper<decltype(struc::field)>::type(), \
    /* nullable */  FullFieldInfoHelper<decltype(struc::field)>::nullable(), \
    /* extension */ FullFieldInfoHelper<decltype(struc::field)>::extension(__VA_ARGS__) \
    },

#define ENUM_INFO_BEGIN(_enum, type, def) \
	extern EnumValueInfoDescriptor _enumValueInfos_##_enum[]; \
	EnumInfoDescriptor _enumInfo_##_enum = { type, #_enum, (uint32_t)def, _enumValueInfos_##_enum }; \
	EnumValueInfoDescriptor _enumValueInfos_##_enum[] = {

#define ENUM_INFO_END(_enum) \
		{ nullptr, 0 } \
	};

#define ENUM_INFO(_enum) _enumInfo_##_enum
#define ENUM_INFO_DECLARE(_enum) extern EnumInfoDescriptor _enumInfo_##_enum;

#define VALUE_INFO(name) \
	{ #name, (uint32_t)name },

#endif // METAINFO_H
