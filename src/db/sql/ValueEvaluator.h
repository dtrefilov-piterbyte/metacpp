#ifndef VALUEEVALUATOR
#define VALUEEVALUATOR
#include "StringBase.h"
#include "Variant.h"

namespace metacpp
{
namespace db
{
namespace sql
{

/** \brief Performs conversion of C++ value to an SQL literal */
template<typename T, typename = void>
struct ValueEvaluator;

template<>
struct ValueEvaluator<String>
{
public:
    String operator()(const String& val) const
    {
        return "\'" + escape(val) + "\'";
    }

private:
    String escape(const String& s) const
    {
        String s1 = s;
        s1.replace("'", "\'\'");
        return s1;
    }
};

template<>
struct ValueEvaluator<DateTime>
{
public:
    String operator()(const DateTime& val) const
    {
        return "\'" + val.toString() + "\'";
    }
};

template<typename T>
struct ValueEvaluator<T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
{
    String operator()(const T& val) const
    {
        return String::fromValue(val);
    }
};

template<>
struct ValueEvaluator<Variant>
{
    String operator()(const Variant& val) const
    {
        if (!val.valid())
            throw std::invalid_argument("Cannot evaluate value of an invalid variant");
        if (val.isArithmetic())
            return variant_cast<String>(val);
        if (val.isString())
        {
            ValueEvaluator<String> ieval;
            return ieval(variant_cast<String>(val));
        }
        if (val.isDateTime())
        {
            ValueEvaluator<DateTime> ieval;
            return ieval(variant_cast<DateTime>(val));
        }
        throw std::invalid_argument("Unsupported variant type");
    }
};

} // namespace sql
} // namespace db
} // namespace metacpp

#endif // VALUEEVALUATOR

