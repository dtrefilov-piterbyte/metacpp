#ifndef TYPEPROMOTION_H
#define TYPEPROMOTION_H
#include <type_traits>

namespace metacpp
{
namespace db
{
namespace detail
{

template<typename T, typename Enable = void>
struct TypePromotionPriorityHelper;

template<>
struct TypePromotionPriorityHelper<long double>
{
    static const int priority = 0;
};

template<>
struct TypePromotionPriorityHelper<double>
{
    static const int priority = 1;
};

template<>
struct TypePromotionPriorityHelper<float>
{
    static const int priority = 2;
};

template<>
struct TypePromotionPriorityHelper<long long unsigned int>
{
    static const int priority = 3;
};

template<>
struct TypePromotionPriorityHelper<long long int>
{
    static const int priority = 4;
};

template<>
struct TypePromotionPriorityHelper<long unsigned int>
{
    static const int priority = 5;
};

template<>
struct TypePromotionPriorityHelper<long int>
{
    static const int priority = 6;
};

template<>
struct TypePromotionPriorityHelper<unsigned int>
{
    static const int priority = 7;
};

template<>
struct TypePromotionPriorityHelper<int>
{
    static const int priority = 8;
};

// all others are promoted to int
template<typename T>
struct TypePromotionPriorityHelper<T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
{
    static const int priority = 1000;
};

enum _PromotionType
{
    _PromoteToFirst,
    _PromoteToSecond,
    _PromoteToInt
};

template<typename T1, typename T2, _PromotionType>
struct TypePromotionHelper;

template<typename T1, typename T2>
struct TypePromotionHelper<T1, T2, _PromoteToFirst>
{
    typedef T1 PromotionType;
};

template<typename T1, typename T2>
struct TypePromotionHelper<T1, T2, _PromoteToSecond>
{
    typedef T2 PromotionType;
};

template<typename T1, typename T2>
struct TypePromotionHelper<T1, T2, _PromoteToInt>
{
    typedef int PromotionType;
};

template<typename TField1, typename TField2>
struct TypePromotion
{
    typedef typename TypePromotionHelper<TField1, TField2,
    TypePromotionPriorityHelper<TField1>::priority >= 1000 &&
    TypePromotionPriorityHelper<TField2>::priority >= 1000 ? _PromoteToInt :
    (TypePromotionPriorityHelper<TField1>::priority <
     TypePromotionPriorityHelper<TField2>::priority ?
         _PromoteToFirst : _PromoteToSecond)>::PromotionType Type;
};

} // namespace detail
} // namespace db
} // namespace metacpp

#endif // TYPEPROMOTION_H

