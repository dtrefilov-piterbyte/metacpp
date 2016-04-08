#ifndef METATYPE
#define METATYPE
#include <type_traits>
#include <cstdint>

/** \brief Type of the fields
 * \relates metacpp::FieldInfoDescriptor
 */
enum EFieldType
{
    eFieldVoid		= 'x',                              /**< \brief void type (invalid) */
    eFieldBool		= 'b',                              /**< \brief bool type */
    eFieldInt		= 'i',                              /**< \brief int32_t type */
    eFieldUint		= 'u',                              /**< \brief uint32_t type */
    eFieldInt64     = 'i' | ('6' << 8) | ('4' << 16),   /**< \brief int64_t type */
    eFieldUint64    = 'u' | ('6' << 8) | ('4' << 16),   /**< \brief uint64_t type */
    eFieldFloat		= 'f',                              /**< \brief float type */
    eFieldDouble    = 'd',                              /**< \brief double type */
    eFieldString	= 's',                              /**< \brief metacpp::String type */
    eFieldDateTime  = 't',                              /**< \brief metacpp::DateTime type */
    eFieldEnum		= 'e',                              /**< \brief enum type */
    eFieldObject	= 'o',                              /**< \brief metacpp::Object type */
    eFieldArray		= 'a',                              /**< \brief metacpp::Array type */
    eFieldVariant   = 'v'                               /**< \brief metacpp::Variant type */
};

/** \brief Parameter determines assigning behaviour of the field
 * \relates metacpp::FieldInfoDescriptor
*/
enum EMandatoriness
{
    eRequired,			/**< \brief an exception is thrown if field value was not excplicitly specified  */
    eOptional,			/**< \brief ignoring omited descriptor */
    eDefaultable		/**< \brief field is assigned to default value */
};

/** \brief Type of enumeration
 * \relates metacpp::EnumInfoDescriptor
*/
enum EEnumType
{
    eEnumNone,        /**< \brief Unknown enumeration type */
    eEnumSimple,      /**< \brief Simple enumeration type */
    eEnumBitset       /**< \brief Enumeration defines a set of bits (bitmask) */
};

namespace metacpp
{
    template<typename T>
    class Array;

    template<typename T>
    class StringBase;

    class Object;
    class DateTime;
    class Variant;
}

template<typename T>
struct MayBeField : std::false_type { };

template<> struct MayBeField<bool> : std::true_type { };
template<> struct MayBeField<int32_t> : std::true_type { };
template<> struct MayBeField<uint32_t> : std::true_type { };
template<> struct MayBeField<int64_t> : std::true_type { };
template<> struct MayBeField<uint64_t> : std::true_type { };
template<> struct MayBeField<float> : std::true_type { };
template<> struct MayBeField<double> : std::true_type { };
template<> struct MayBeField<metacpp::StringBase<char> > : std::true_type { };
template<> struct MayBeField<metacpp::DateTime> : std::true_type { };
template<typename TElem>
struct MayBeField<metacpp::Array<TElem> > : std::true_type { };
template<> struct MayBeField<metacpp::Variant> : std::true_type { };



#endif // METATYPE

