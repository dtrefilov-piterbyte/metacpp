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
#include "String.h"
#include "Array.h"
#include "SharedDataBase.h"
#include "MetaType.h"

enum EFieldType;

namespace metacpp
{

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
    
    EFieldType type() const;
    template<typename T>
    T value() const;
    
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
};

/**
    \brief Variant type for storing scalar values.
*/
class Variant : SharedDataPointer<VariantData>
{
public:
    Variant(void);
    ~Variant();

    Variant(bool v);
    Variant(int32_t v);
    Variant(uint32_t v);
    Variant(const int64_t& v);
    Variant(const uint64_t& v);
    Variant(const float& v);
    Variant(const double& v);
    Variant(const char *v);
    Variant(const String& v);
    Variant(const DateTime& v);

    inline EFieldType type() const { return getData()->type(); }

    template<typename T>
    typename std::enable_if<!std::is_void<T>::value && !std::is_same<Variant, T>::value, T>::type value() const
    {
        return getData()->value<T>();
    }

    template<typename T>
    typename std::enable_if<std::is_void<T>::value, void>::type value() const
    {
        if (valid()) throw std::runtime_error("Not a void variant");
    }

    template<typename T>
    typename std::enable_if<std::is_same<T, Variant>::value, const Variant&>::type value() const
    {
        return *this;
    }

    bool valid() const;
    bool isIntegral() const;
    bool isFloatingPoint() const;
    bool isArithmetic() const;
    bool isString() const;
    bool isDateTime() const;
private:
    VariantData *getData() const;
};

/** \brief Gets the stored value.
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

} // namespace metacpp

#endif // VARIANT_H
