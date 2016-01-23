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
#ifndef VARIANT_H
#define VARIANT_H
#include "DateTime.h"
#include "StringBase.h"
#include "Array.h"
#include "SharedDataBase.h"
#include "MetaType.h"

enum EFieldType;

namespace metacpp
{

class Object;
class Variant;

namespace detail
{

    template<typename TObj>
    struct IsObjectPtr : std::false_type
    {
    };

    template<typename TObj>
    struct IsObjectPtr<TObj *> : std::integral_constant<bool,
               std::is_same<typename std::remove_cv<TObj>::type, Object>::value ||
               std::is_base_of<Object, typename std::remove_cv<TObj>::type>::value>
    {
    };

    class VariantData : public SharedDataBase
    {
    public:
        VariantData();
        ~VariantData();

        explicit VariantData(bool v);
        explicit VariantData(int32_t v);
        explicit VariantData(uint32_t v);
        explicit VariantData(const int64_t& v);
        explicit VariantData(const uint64_t& v);
        explicit VariantData(const float& v);
        explicit VariantData(const double& v);
        explicit VariantData(const String& v);
        explicit VariantData(const DateTime& v);
        explicit VariantData(Object *o);
        explicit VariantData(const Array<Variant>& a);

        EFieldType type() const;
        template<typename T>
        T value() const;
        void *buffer();

        SharedDataBase *clone() const override;
    private:
        template<typename T>
        T arithmetic_convert() const;
    private:
        EFieldType m_type;
        // union storage for POD types
        union
        {
            bool m_bool;
            int32_t m_int;
            uint32_t m_uint;
            int64_t m_int64;
            uint64_t m_uint64;
            float m_float;
            double m_double;
        } m_storage;
        String m_string;
        DateTime m_datetime;
        Array<Variant> m_array;
        SharedObjectPointer<Object> m_object;
    };
} // namespace detail

/**
    \brief Class for effective union-like storing scalar values of several types.

    Supported types:
    - void, invalid variant (eFieldVoid)
    - bool (eFieldBool)
    - int32_t (eFieldInt)
    - uint32_t (eFieldUint)
    - int64_t (eFieldInt64)
    - uint64_t (eFieldUint64)
    - float (eFieldFloat)
    - double (eFieldDouble)
    - metacpp::String (eFieldString)
    - metacpp::DateTime (eFieldDateTime)
    - metacpp::Object * (eFieldObject)
    - metacpp::Array<metacpp::Variant> (eFieldArray)
*/
class Variant final : SharedDataPointer<detail::VariantData>
{
public:
    /** \brief Constructs a new instance of the void (invalid) variant */
    Variant(void);
    ~Variant();

    /** \brief Constructs a new instance of the bool variant */
    Variant(bool v);
    /** \brief Constructs a new instance of the int32_t variant */
    Variant(int32_t v);
    /** \brief Constructs a new instance of the uint32_t variant */
    Variant(uint32_t v);
    /** \brief Constructs a new instance of the int64_t variant */
    Variant(const int64_t& v);
    /** \brief Constructs a new instance of the uint64_t variant */
    Variant(const uint64_t& v);
    /** \brief Constructs a new instance of the float variant */
    Variant(const float& v);
    /** \brief Constructs a new instance of the double variant */
    Variant(const double& v);
    /** \brief Constructs a new instance of the metacpp::String variant using a pointer to C-style string */
    Variant(const char *v);
    /** \brief Constructs a new instance of the metacpp::String variant */
    Variant(const String& v);
    /** \brief Constructs a new instance of the metacpp::DateTime variant */
    Variant(const DateTime& v);
    /** \brief Constructs a new instance of the metacpp::Object * variant.
     * Variant will take the ownership and will be responsible to delete the object with
     * default deleter
    */
    Variant(Object *o);
    /** \brief Constructs a new instance of the VariantArray variant */
    Variant(const Array<Variant>& a);

    /** \brief Gets a type of the stored value */
    inline EFieldType type() const { return getData()->type(); }

    /** \brief Gets the stored value of converted to the type T if needed.
     *
     * \see variant_cast
    */
    template<typename T>
    typename std::enable_if<!std::is_void<T>::value &&
                            !std::is_same<Variant, T>::value &&
                            !detail::IsObjectPtr<T>::value, T>::type value() const
    {
        return getData()->value<T>();
    }

    template<typename T>
    typename std::enable_if<detail::IsObjectPtr<T>::value, T>::type value() const
    {
        Object *o = getData()->value<Object *>();
        if (o)
        {
            T derived = dynamic_cast<T>(o);
            if (!derived) throw std::bad_cast();
            return derived;
        }
        return nullptr;
    }

    /** \brief Ensures that this variant is invalid (i.e. of the void type) */
    template<typename T>
    typename std::enable_if<std::is_void<T>::value, void>::type value() const
    {
        if (valid()) throw std::runtime_error("Not a void variant");
    }

    /** \brief Specialization for the getter of stored value of unspecified type */
    template<typename T>
    typename std::enable_if<std::is_same<T, Variant>::value, const Variant&>::type value() const
    {
        return *this;
    }

    /** \brief Checks if this variant is valid (i.e. of non-void type) */
    bool valid() const;
    /** \brief Checks whether this variant stores value of any integral type (bool, int32_t, uint32_t, int64_t or uint64_t) */
    bool isIntegral() const;
    /** \brief Checks whether this variant stores value of any floating point type (float or double) */
    bool isFloatingPoint() const;
    /** \brief Checks whether this variant stores a number (i.e. integral or floating point)
     * \see isIntegral, isFloatingPoint
    */
    bool isArithmetic() const;
    /** \brief Checks if this variant stores a metacpp::String */
    bool isString() const;
    /** \brief Checks if this variant stores a metacpp::DateTime */
    bool isDateTime() const;
    /** \brief Checks if this variant stores a metacpp::Object * */
    bool isObject() const;
    /** \brief Checks if this variant stores a VariantArray * */
    bool isArray() const;

    const void *buffer() const;
private:
    detail::VariantData *getData() const;
};

/** \brief Generalized form of getter for the stored value of v to the template type T.
An automatic type conversion is performed as needed:
 - any arithmetic (either integral or floating point) is converted to any other arithmetic type
 - any type is converted to String using String::fromValue()
*/
template<typename T>
T variant_cast(const Variant& v)
{
    return v.value<T>();
}

typedef Array<Variant> VariantArray;

std::basic_ostream<char>& operator<<(std::basic_ostream<char>& stream, const Variant& v);
std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& stream, const Variant& v);

} // namespace metacpp

#endif // VARIANT_H
