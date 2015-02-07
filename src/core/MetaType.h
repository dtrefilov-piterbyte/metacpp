#ifndef METATYPE
#define METATYPE

enum EFieldType
{
    eFieldVoid		= 'x',
    eFieldBool		= 'b',
    eFieldInt		= 'i',
    eFieldUint		= 'u',
    eFieldInt64     = 'i' | ('6' << 8) | ('4' << 16),
    eFieldUint64    = 'u' | ('6' << 8) | ('4' << 16),
    eFieldFloat		= 'f',
    eFieldDouble    = 'd',
    eFieldString	= 's',
    eFieldEnum		= 'e',
    eFieldDateTime  = 't',
    eFieldObject	= 'o',
    eFieldArray		= 'a',
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

#endif // METATYPE

