#ifndef SQLWHERECLAUSE_H
#define SQLWHERECLAUSE_H
#include <type_traits>
#include "Object.h"

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
    virtual String expression() const = 0;
    virtual bool complex() const { return false; }
};

/** \brief Builder with prepared conditional expression */
class DirectWhereClauseBuilder : public WhereClauseBuilder
{
public:
    DirectWhereClauseBuilder(const String& s);

    ~DirectWhereClauseBuilder();

    String expression() const override;

private:
    String m_string;
};

/** Where clause compined from two others using 'and' or 'or' operators */
class ComplexWhereClauseBuilder : public DirectWhereClauseBuilder
{
public:
    enum Operator
    {
        OperatorAnd,
        OperatorOr
    };

    ComplexWhereClauseBuilder(Operator op, const WhereClauseBuilder& left,
                              const WhereClauseBuilder& right);

    bool complex() const override
    {
        return true;
    }
private:
    String buildExpression(Operator op, const WhereClauseBuilder& left,
                              const WhereClauseBuilder& right);
};

class NegationWhereClauseBuilder : public DirectWhereClauseBuilder
{
public:
    NegationWhereClauseBuilder(const WhereClauseBuilder& inner);

    String buildExpression(const WhereClauseBuilder& inner);
};

inline ComplexWhereClauseBuilder operator &&(const WhereClauseBuilder& left,
                                      const WhereClauseBuilder& right)
{
    return ComplexWhereClauseBuilder(ComplexWhereClauseBuilder::OperatorAnd, left, right);
}

inline ComplexWhereClauseBuilder operator ||(const WhereClauseBuilder& left,
                                      const WhereClauseBuilder& right)
{
    return ComplexWhereClauseBuilder(ComplexWhereClauseBuilder::OperatorOr, left, right);
}

inline NegationWhereClauseBuilder operator !(const WhereClauseBuilder& inner)
{
    return NegationWhereClauseBuilder(inner);
}

} // namespace sql
} // namespace metacpp

#endif // SQLWHERECLAUSE_H
