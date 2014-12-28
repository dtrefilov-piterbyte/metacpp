#ifndef SQLWHERECLAUSE_H
#define SQLWHERECLAUSE_H
#include <type_traits>
#include "Object.h"
#include "CDebug.h"

namespace metacpp
{
namespace sql
{

/** \brief Basic builder for sql conditional expressions (usually in where clause) */
class WhereClauseBuilder
{
public:
    virtual ~WhereClauseBuilder() { }

    /** \brief print sql subexpression into a stream */
    virtual std::ostream& buildStatement(std::ostream& os) const = 0;
    virtual bool complex() const { return false; }
};

/** \brief Builder with prepared conditional expression */
class ExplicitWhereClauseBuilder : public WhereClauseBuilder
{
public:
    ExplicitWhereClauseBuilder(const String& s)
        : m_string(s)
    {
    }

    ~ExplicitWhereClauseBuilder()
    {
    }

    std::ostream& buildStatement(std::ostream& os) const override
    {
        return os << m_string;
    }

private:
    String m_string;
};

/** Where clause compined from two others using 'and' or 'or' operators */
class ComplexWhereClauseBuilder : public WhereClauseBuilder
{
public:
    enum Operator
    {
        OperatorAnd,
        OperatorOr
    };

    ComplexWhereClauseBuilder(Operator op, const WhereClauseBuilder& left,
                              const WhereClauseBuilder& right)
        : m_operator(op), m_left(left), m_right(right)
    {
    }

    std::ostream& buildStatement(std::ostream& os) const override
    {
        if (m_left.complex()) os << "("; // parenthesis
        m_left.buildStatement(os);
        if (m_left.complex()) os << ")"; // parenthesis

        switch (m_operator)
        {
        case OperatorAnd:
            os << " AND ";
            break;
        case OperatorOr:
            os << " OR ";
            break;
        default:
            throw std::invalid_argument("Wrong operator");
        }

        if (m_right.complex()) os << "("; // parenthesis
        m_right.buildStatement(os);
        if (m_right.complex()) os << ")"; // parenthesis
    }

    bool complex() const override
    {
        return true;
    }
private:
    Operator m_operator;
    const WhereClauseBuilder& m_left;
    const WhereClauseBuilder& m_right;
};

ComplexWhereClauseBuilder operator &&(const WhereClauseBuilder& left,
                                      const WhereClauseBuilder& right)
{
    return ComplexWhereClauseBuilder(ComplexWhereClauseBuilder::OperatorAnd, left, right);
}

ComplexWhereClauseBuilder operator ||(const WhereClauseBuilder& left,
                                      const WhereClauseBuilder& right)
{
    return ComplexWhereClauseBuilder(ComplexWhereClauseBuilder::OperatorOr, left, right);
}

/**
    \brief Basic column matcher
*/
template<typename TObj, typename TField>
class SqlColumnMatcherBase
{
protected:
    explicit SqlColumnMatcherBase(const MetaField *metaField)
        : m_metaField(metaField)
    {
    }

    const MetaField *metaField() const { return m_metaField; }

private:
    const MetaField *m_metaField;
};

/**
  Column matcher supporting standard relational operators: <, <=, >, >=, ==, !=
*/
template<typename TObj, typename TField, typename TValueEvaluator>
struct SqlColumnExpressionMatcher : public SqlColumnMatcherBase<TObj, TField>
{
public:
    SqlColumnExpressionMatcher(const MetaField *metaField)
        : SqlColumnMatcherBase<TObj, TField>(metaField)
    {
    }

    ExplicitWhereClauseBuilder operator <(const TField& val) const { return helper_val(val, " < "); }
    ExplicitWhereClauseBuilder operator <=(const TField& val) const { return helper_val(val, " <= "); }
    ExplicitWhereClauseBuilder operator >(const TField& val) const { return helper_val(val, " > "); }
    ExplicitWhereClauseBuilder operator >=(const TField& val) const { return helper_val(val, " >= "); }
    ExplicitWhereClauseBuilder operator ==(const TField& val) const { return helper_val(val, " = "); }
    ExplicitWhereClauseBuilder operator !=(const TField& val) const { return helper_val(val, " != "); }

    // Operators for JOINs
    // TODO: traits? will kill compile times?
    template<typename TObj1, typename TField1>
    ExplicitWhereClauseBuilder operator <(const SqlColumnMatcherBase<TObj1, TField1>& other) const { return helper_col(other, " < "); }
    template<typename TObj1, typename TField1>
    ExplicitWhereClauseBuilder operator <=(const SqlColumnMatcherBase<TObj1, TField1>& other) const { return helper_col(other, " <= "); }
    template<typename TObj1, typename TField1>
    ExplicitWhereClauseBuilder operator >(const SqlColumnMatcherBase<TObj1, TField1>& other) const { return helper_col(other, " > "); }
    template<typename TObj1, typename TField1>
    ExplicitWhereClauseBuilder operator >=(const SqlColumnMatcherBase<TObj1, TField1>& other) const { return helper_col(other, " >= "); }
    template<typename TObj1, typename TField1>
    ExplicitWhereClauseBuilder operator ==(const SqlColumnMatcherBase<TObj1, TField1>& other) const { return helper_col(other, " = "); }
    template<typename TObj1, typename TField1>
    ExplicitWhereClauseBuilder operator !=(const SqlColumnMatcherBase<TObj1, TField1>& other) const { return helper_col(other, " != "); }
private:
    ExplicitWhereClauseBuilder helper_val(const TField& val, const char *comp) const
    {
        TValueEvaluator eval;
        return ExplicitWhereClauseBuilder(String(TObj::staticMetaObject()->name()) + "." +
            this->metaField()->name() + comp + eval(val));
    }

    template<typename TObj1, typename TField1>
    ExplicitWhereClauseBuilder helper_col(const SqlColumnMatcherBase<TObj1, TField1>& other, const char *comp) const
    {
        return ExplicitWhereClauseBuilder(String(TObj::staticMetaObject()->name()) + "." + this->metaField()->name() + comp +
            TObj1::staticMetaObject()->name() + "." + other->metaField()->name());

    }
};

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
    std::string operator()(int val) const
    {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }
};

template<typename TObj, typename TField, typename Enable = void>
class SqlColumnPartialMatcher;

template<typename TObj>
class SqlColumnPartialMatcher<TObj, String> :
        public SqlColumnExpressionMatcher<TObj, String, ValueEvaluator<String> >
{
public:
    explicit SqlColumnPartialMatcher(const MetaField *metaField)
        : SqlColumnExpressionMatcher<TObj, String, ValueEvaluator<String> >(metaField)
    {
    }
};

template<typename TObj, typename TField>
class SqlColumnPartialMatcher<TObj, TField, typename std::enable_if<std::is_arithmetic<TField>::value>::type> :
        public SqlColumnExpressionMatcher<TObj, TField, ValueEvaluator<TField> >
{
public:
    explicit SqlColumnPartialMatcher(const MetaField *metaField)
        : SqlColumnExpressionMatcher<TObj, TField, ValueEvaluator<TField> >(metaField)
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

} // namespace sql
} // namespace metacpp

#endif // SQLWHERECLAUSE_H
