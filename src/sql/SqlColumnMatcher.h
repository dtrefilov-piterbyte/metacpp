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
    std::string operator()(const T& val) const
    {
        std::ostringstream ss;
        ss << val;
        return ss.str();
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
};

template<typename TObj, typename TField>
SqlColumnFullMatcher<TObj, TField> GetColumnMatcher(const TField TObj::*member)
{
    return SqlColumnFullMatcher<TObj, TField>(member);
}

#define COLUMN(Table, Column) GetColumnMatcher(&Table::Column)

/** Typed expression represented as a raw expression string (TODO: bad practice?) */
template<typename TField>
class SqlColumnMatcherExplicitExpression : public SqlColumnMatcherSubexpression<TField>
{
public:
    SqlColumnMatcherExplicitExpression(const String& expr)
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

/** relational operators forming final where clause */
template<typename TField>
ExplicitWhereClauseBuilder operator <(const SqlColumnMatcherSubexpression<TField>& lhs,
                                      const SqlColumnMatcherSubexpression<TField>& rhs)
{
    return ExplicitWhereClauseBuilder(lhs.expression() + " < " + rhs.expression());
}

template<typename TField>
ExplicitWhereClauseBuilder operator <=(const SqlColumnMatcherSubexpression<TField>& lhs,
                                      const SqlColumnMatcherSubexpression<TField>& rhs)
{
    return ExplicitWhereClauseBuilder(lhs.expression() + " <= " + rhs.expression());
}

template<typename TField>
ExplicitWhereClauseBuilder operator >(const SqlColumnMatcherSubexpression<TField>& lhs,
                                      const SqlColumnMatcherSubexpression<TField>& rhs)
{
    return ExplicitWhereClauseBuilder(lhs.expression() + " > " + rhs.expression());
}

template<typename TField>
ExplicitWhereClauseBuilder operator >=(const SqlColumnMatcherSubexpression<TField>& lhs,
                                      const SqlColumnMatcherSubexpression<TField>& rhs)
{
    return ExplicitWhereClauseBuilder(lhs.expression() + " >= " + rhs.expression());
}

template<typename TField>
ExplicitWhereClauseBuilder operator ==(const SqlColumnMatcherSubexpression<TField>& lhs,
                                      const SqlColumnMatcherSubexpression<TField>& rhs)
{
    return ExplicitWhereClauseBuilder(lhs.expression() + " = " + rhs.expression());
}

template<typename TField>
ExplicitWhereClauseBuilder operator !=(const SqlColumnMatcherSubexpression<TField>& lhs,
                                      const SqlColumnMatcherSubexpression<TField>& rhs)
{
    return ExplicitWhereClauseBuilder(lhs.expression() + " < " + rhs.expression());
}

template<typename TField>
ExplicitWhereClauseBuilder operator <(const TField& lhs,
                                      const SqlColumnMatcherSubexpression<TField>& rhs)
{
    ValueEvaluator<TField> eval;
    return ExplicitWhereClauseBuilder(eval(lhs) + " < " + rhs.expression());
}

template<typename TField>
ExplicitWhereClauseBuilder operator <=(const TField& lhs,
                                      const SqlColumnMatcherSubexpression<TField>& rhs)
{
    ValueEvaluator<TField> eval;
    return ExplicitWhereClauseBuilder(eval(lhs) + " <= " + rhs.expression());
}

template<typename TField>
ExplicitWhereClauseBuilder operator >(const TField& lhs,
                                      const SqlColumnMatcherSubexpression<TField>& rhs)
{
    ValueEvaluator<TField> eval;
    return ExplicitWhereClauseBuilder(eval(lhs) + " > " + rhs.expression());
}

template<typename TField>
ExplicitWhereClauseBuilder operator >=(const TField& lhs,
                                      const SqlColumnMatcherSubexpression<TField>& rhs)
{
    ValueEvaluator<TField> eval;
    return ExplicitWhereClauseBuilder(eval(lhs) + " >= " + rhs.expression());
}

template<typename TField>
ExplicitWhereClauseBuilder operator ==(const TField& lhs,
                                      const SqlColumnMatcherSubexpression<TField>& rhs)
{
    ValueEvaluator<TField> eval;
    return ExplicitWhereClauseBuilder(eval(lhs) + " = " + rhs.expression());
}

template<typename TField>
ExplicitWhereClauseBuilder operator !=(const TField& lhs,
                                      const SqlColumnMatcherSubexpression<TField>& rhs)
{
    ValueEvaluator<TField> eval;
    return ExplicitWhereClauseBuilder(eval(lhs) + " < " + rhs.expression());
}


template<typename TField>
ExplicitWhereClauseBuilder operator <(const SqlColumnMatcherSubexpression<TField>& lhs,
                                      const TField& rhs)
{
    ValueEvaluator<TField> eval;
    return ExplicitWhereClauseBuilder(lhs.expression() + " < " + eval(rhs));
}

template<typename TField>
ExplicitWhereClauseBuilder operator <=(const SqlColumnMatcherSubexpression<TField>& lhs,
                                      const TField& rhs)
{
    ValueEvaluator<TField> eval;
    return ExplicitWhereClauseBuilder(lhs.expression() + " <= " + eval(rhs));
}

template<typename TField>
ExplicitWhereClauseBuilder operator >(const SqlColumnMatcherSubexpression<TField>& lhs,
                                      const TField& rhs)
{
    ValueEvaluator<TField> eval;
    return ExplicitWhereClauseBuilder(lhs.expression() + " > " + eval(rhs));
}

template<typename TField>
ExplicitWhereClauseBuilder operator >=(const SqlColumnMatcherSubexpression<TField>& lhs,
                                      const TField& rhs)
{
    ValueEvaluator<TField> eval;
    return ExplicitWhereClauseBuilder(lhs.expression() + " >= " + eval(rhs));
}

template<typename TField>
ExplicitWhereClauseBuilder operator ==(const SqlColumnMatcherSubexpression<TField>& lhs,
                                      const TField& rhs)
{
    ValueEvaluator<TField> eval;
    return ExplicitWhereClauseBuilder(lhs.expression() + " = " + eval(rhs));
}

template<typename TField>
ExplicitWhereClauseBuilder operator !=(const SqlColumnMatcherSubexpression<TField>& lhs,
                                      const TField& rhs)
{
    ValueEvaluator<TField> eval;
    return ExplicitWhereClauseBuilder(lhs.expression() + " < " + eval(rhs));
}


/** unary - */
template<typename TField>
SqlColumnMatcherExplicitExpression<TField>
operator-(const SqlColumnMatcherSubexpression<TField>& inner)
{
    return SqlColumnMatcherExplicitExpression<TField>("(-" + inner.expression() + ")");
}

} // namespace sql
} // namespace metacpp

#endif // SQLCOLUMNMATCHER

