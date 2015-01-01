#ifndef SQLCOLUMNMATCHER
#define SQLCOLUMNMATCHER
#include "String.h"
#include "SqlWhereClause.h"

namespace metacpp
{
namespace sql
{

/** Performs conversion of C++ value to SQL literal */
template<typename T, typename = void>
struct ValueEvaluator;

template<typename TObj1, typename TField1, typename TField2>
class SqlColumnAssignment;

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
        // TODO: escape single quote
        return s;
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

template<typename TField>
class SqlColumnMatcherSubexpression
{
public:
    virtual String expression() const = 0;
};

/** value expression */
template<typename TField>
class SqlColumnMatcherValue : public SqlColumnMatcherSubexpression<TField>
{
public:
    SqlColumnMatcherValue(const TField& val)
        : m_val(val)
    {
    }

    ~SqlColumnMatcherValue()
    {
    }

    String expression() const override
    {
        ValueEvaluator<TField> eval;
        return eval(m_val);
    }

private:
    TField m_val;
};

/**
    \brief Basic column matcher
*/
template<typename TObj, typename TField>
class SqlColumnMatcherFieldBase : public SqlColumnMatcherSubexpression<TField>
{
public:
    explicit SqlColumnMatcherFieldBase(const MetaField *metaField)
        : m_metaField(metaField)
    {
    }

    const MetaField *metaField() const { return m_metaField; }

    String expression() const override
    {
        return String(TObj::staticMetaObject()->name()) + "." + this->metaField()->name();
    }


private:
    const MetaField *m_metaField;
};

/** Typed expression represented as a raw expression string (TODO: bad practice?) */
template<typename TField>
class SqlColumnMatcherExplicitExpression : public SqlColumnMatcherSubexpression<TField>
{
public:
    explicit SqlColumnMatcherExplicitExpression(const String& expr)
        : m_expr(expr)
    {

    }

    ~SqlColumnMatcherExplicitExpression()
    {
    }

    String expression() const override
    {
        return m_expr;
    }
private:
    String m_expr;
};

template<typename TObj, typename TField, typename Enable = void>
class SqlColumnPartialMatcher;

template<typename TObj>
class SqlColumnPartialMatcher<TObj, String> :
        public SqlColumnMatcherFieldBase<TObj, String>
{
public:
    explicit SqlColumnPartialMatcher(const MetaField *metaField)
        : SqlColumnMatcherFieldBase<TObj, String>(metaField)
    {
    }

    ExplicitWhereClauseBuilder like(const String& val) const
    {
        return ExplicitWhereClauseBuilder(this->expression() + " LIKE \'" + val + "\'");
    }

    // TODO: lower, upper, trimming functions etc.
};

template<typename TObj, typename TField>
class SqlColumnPartialMatcher<TObj, TField, typename std::enable_if<std::is_arithmetic<TField>::value>::type> :
        public SqlColumnMatcherFieldBase<TObj, TField>
{
public:
    explicit SqlColumnPartialMatcher(const MetaField *metaField)
        : SqlColumnMatcherFieldBase<TObj, TField>(metaField)
    {
    }
};

template<typename TObj, typename TField>
class SqlColumnFullMatcher
        : public SqlColumnPartialMatcher<TObj, TField>
{
public:
    explicit SqlColumnFullMatcher(const TField TObj::*member)
        : SqlColumnPartialMatcher<TObj, TField>(getMetaField(member))
    {
    }

    template<typename TField2>
    typename std::enable_if<std::is_convertible<TField2, TField>::value, SqlColumnAssignment<TObj, TField, TField2>>::type
    operator=(const SqlColumnMatcherSubexpression<TField2>& rhs)
    {
        return SqlColumnAssignment<TObj, TField, TField2>(*this, rhs);
    }

    template<typename TField2>
    typename std::enable_if<std::is_convertible<TField2, TField>::value, SqlColumnAssignment<TObj, TField, TField2>>::type
    operator=(const TField2& rhs)
    {
        return SqlColumnAssignment<TObj, TField, TField2>(*this, SqlColumnMatcherValue<TField2>(rhs));
    }
};


template<typename TObj, typename TField>
class SqlColumnFullMatcher<TObj, Nullable<TField> >
        : public SqlColumnPartialMatcher<TObj, TField>
{
public:
    explicit SqlColumnFullMatcher(const Nullable<TField> TObj::*member)
        : SqlColumnPartialMatcher<TObj, TField>(getMetaField(member))
    {
    }

    ExplicitWhereClauseBuilder isNull() const
    {
        return ExplicitWhereClauseBuilder(String(TObj::staticMetaObject()->name()) + "." +
            this->metaField()->name() + " IS NULL");
    }

    ExplicitWhereClauseBuilder isNotNull() const
    {
        return ExplicitWhereClauseBuilder(String(TObj::staticMetaObject()->name()) + "." +
            this->metaField()->name() + " IS NOT NULL");
    }

    template<typename TField2>
    typename std::enable_if<std::is_convertible<TField2, TField>::value, SqlColumnAssignment<TObj, TField, TField2>>::type
    operator=(const SqlColumnMatcherSubexpression<TField2>& rhs)
    {
        return SqlColumnAssignment<TObj, TField, TField2>(*this, rhs);
    }

    template<typename TField2>
    typename std::enable_if<std::is_convertible<TField2, TField>::value, SqlColumnAssignment<TObj, TField, TField2>>::type
    operator=(const TField2& rhs)
    {
        return SqlColumnAssignment<TObj, TField, TField2>(*this, SqlColumnMatcherValue<TField2>(rhs));
    }

    SqlColumnAssignment<TObj, TField, TField> operator=(std::nullptr_t)
    {
        return SqlColumnAssignment<TObj, TField, TField>(*this, SqlColumnMatcherExplicitExpression<TField>("NULL"));
    }
};

template<typename TObj, typename TField>
SqlColumnFullMatcher<TObj, TField> GetColumnMatcher(const TField TObj::*member)
{
    return SqlColumnFullMatcher<TObj, TField>(member);
}

#define COLUMN(Table, Column) GetColumnMatcher(&Table::Column)

template<typename T, typename Enable = void>
struct TypePromotionPriorityHelper;

template<>
struct TypePromotionPriorityHelper<long double>
{
    static constexpr int priority() { return 0; }
};

template<>
struct TypePromotionPriorityHelper<double>
{
    static constexpr int priority() { return 1; }
};

template<>
struct TypePromotionPriorityHelper<float>
{
    static constexpr int priority() { return 2; }
};

template<>
struct TypePromotionPriorityHelper<long long unsigned int>
{
    static constexpr int priority() { return 3; }
};

template<>
struct TypePromotionPriorityHelper<long long int>
{
    static constexpr int priority() { return 4; }
};

template<>
struct TypePromotionPriorityHelper<long unsigned int>
{
    static constexpr int priority() { return 5; }
};

template<>
struct TypePromotionPriorityHelper<long int>
{
    static constexpr int priority() { return 6; }
};

template<>
struct TypePromotionPriorityHelper<unsigned int>
{
    static constexpr int priority() { return 7; }
};

template<>
struct TypePromotionPriorityHelper<int>
{
    static constexpr int priority() { return 8; }
};

// all others are promoted to int
template<typename T>
struct TypePromotionPriorityHelper<T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
{
    static constexpr int priority() { return 1000; }
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
    TypePromotionPriorityHelper<TField1>::priority() >= 1000 &&
    TypePromotionPriorityHelper<TField2>::priority() >= 1000 ? _PromoteToInt :
    (TypePromotionPriorityHelper<TField1>::priority() <
     TypePromotionPriorityHelper<TField2>::priority() ?
         _PromoteToFirst : _PromoteToSecond)>::PromotionType Type;
};

//static_assert(std::is_same<typename TypePromotion<int, char>::Type, int>::value, "int with char should promote to int");
//static_assert(std::is_same<typename TypePromotion<short, char>::Type, int>::value, "short with char should promote to int");
//static_assert(std::is_same<typename TypePromotion<short, float>::Type, float>::value, "short with float should promote to float");
//static_assert(std::is_same<typename TypePromotion<float, float>::Type, float>::value, "float with float should promote to float");

#define _INST_REL_OPERATOR(op, sqlop) \
    template<typename TField1, typename TField2> \
    typename std::enable_if<std::is_same<TField1, TField2>::value || \
    (std::is_arithmetic<TField1>::value && \
    std::is_arithmetic<TField2>::value), ExplicitWhereClauseBuilder>::type \
    operator op(const SqlColumnMatcherSubexpression<TField1>& lhs, \
               const SqlColumnMatcherSubexpression<TField2>& rhs) \
    { \
        return ExplicitWhereClauseBuilder(lhs.expression() + " " + #sqlop + " " + rhs.expression()); \
    } \
    \
    template<typename TField1, typename TField2> \
    typename std::enable_if<std::is_same<TField1, TField2>::value || \
    (std::is_arithmetic<TField1>::value && \
    std::is_arithmetic<TField2>::value), ExplicitWhereClauseBuilder>::type \
    operator op(const TField1& lhs, \
               const SqlColumnMatcherSubexpression<TField2>& rhs) \
    { \
        ValueEvaluator<TField1> eval;\
        return ExplicitWhereClauseBuilder(eval(lhs) + " " #sqlop " " + rhs.expression()); \
    } \
    template<typename TField1, typename TField2> \
    typename std::enable_if<std::is_same<TField1, TField2>::value || \
    (std::is_arithmetic<TField1>::value && \
    std::is_arithmetic<TField2>::value), ExplicitWhereClauseBuilder>::type \
    operator op(const SqlColumnMatcherSubexpression<TField1>& lhs, \
               const TField2& rhs) \
    { \
        ValueEvaluator<TField2> eval;\
        return ExplicitWhereClauseBuilder(lhs.expression() + " " + #sqlop + " " + eval(rhs)); \
    } \

_INST_REL_OPERATOR(==, =)
_INST_REL_OPERATOR(!=, !=)
_INST_REL_OPERATOR(>=, >=)
_INST_REL_OPERATOR(<=, <=)
_INST_REL_OPERATOR(>, >)
_INST_REL_OPERATOR(<, <)

#undef _INST_REL_OPERATOR

/** unary - */
template<typename TField>
SqlColumnMatcherExplicitExpression<TField>
operator-(const SqlColumnMatcherSubexpression<TField>& inner)
{
    return SqlColumnMatcherExplicitExpression<TField>("(-" + inner.expression() + ")");
}

/** unary + */
template<typename TField>
SqlColumnMatcherExplicitExpression<TField>
operator+(const SqlColumnMatcherSubexpression<TField>& inner)
{
    return SqlColumnMatcherExplicitExpression<TField>("(+" + inner.expression() + ")");
}

#define _INST_BINARY_OPERATOR(op) \
template<typename TField1, typename TField2, typename = typename std::enable_if<std::is_arithmetic<TField1>::value && std::is_arithmetic<TField2>::value>::type> \
SqlColumnMatcherExplicitExpression<typename TypePromotion<TField1, TField2>::Type> \
operator op(const SqlColumnMatcherSubexpression<TField1>& lhs, \
          const TField2& rhs) \
{ \
    ValueEvaluator<TField2> eval; \
    return SqlColumnMatcherExplicitExpression<typename TypePromotion<TField1, TField2>::Type>( \
        "(" + lhs.expression() + " " #op " " + eval(rhs) + ")"); \
} \
template<typename TField1, typename TField2, typename = typename std::enable_if<std::is_arithmetic<TField1>::value && std::is_arithmetic<TField2>::value>::type> \
SqlColumnMatcherExplicitExpression<typename TypePromotion<TField1, TField2>::Type> \
operator op(const TField1& lhs, \
          const SqlColumnMatcherSubexpression<TField2>& rhs) \
{ \
    ValueEvaluator<TField1> eval; \
    return SqlColumnMatcherExplicitExpression<typename TypePromotion<TField1, TField2>::Type>("(" + eval(lhs) + " " #op " " + rhs.expression() + ")"); \
} \
template<typename TField1, typename TField2, typename = typename std::enable_if<std::is_arithmetic<TField1>::value && std::is_arithmetic<TField2>::value>::type> \
SqlColumnMatcherExplicitExpression<typename TypePromotion<TField1, TField2>::Type> \
operator op(const SqlColumnMatcherSubexpression<TField1>& lhs, \
          const SqlColumnMatcherSubexpression<TField2>& rhs) \
{ \
    return SqlColumnMatcherExplicitExpression<typename TypePromotion<TField1, TField2>::Type>( \
                "(" + lhs.expression() + " " #op + " " + rhs.expression() + ")"); \
}

_INST_BINARY_OPERATOR(+)
_INST_BINARY_OPERATOR(-)
_INST_BINARY_OPERATOR(*)
_INST_BINARY_OPERATOR(/)
_INST_BINARY_OPERATOR(%)


#undef _INST_BINARY_OPERATOR

} // namespace sql
} // namespace metacpp

#endif // SQLCOLUMNMATCHER

