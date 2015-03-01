#ifndef METACPP_DB_EXPRESSIONWHERECLAUSE_H
#define METACPP_DB_EXPRESSIONWHERECLAUSE_H
#include <memory>
#include "StringBase.h"

namespace metacpp {
namespace db {

class ExpressionNodeBase;
class ExpressionWhereClause;

/** \brief Type of a where clause subexpression */
enum WhereClauseExpressionType
{
    eWhereClauseRelational,
    eWhereClauseUnaryLogical,
    eWhereClauseComplex
};

/** \brief Types of binary relational operators used for building where clauses from two ExpressionNode objects */
enum RelationOperatorType
{
    eRelationalOperatorEqual,
    eRelationalOperatorNotEqual,
    eRelationalOperatorLess,
    eRelationalOperatorLessOrEqual,
    eRelationalOperatorGreater,
    eRelationalOperatorGreaterOrEqual,
    eRelationalOperatorLike,
    eRelationalOperatorIsNull,
    eRelationalOperatorIsNotNull
};

/** \brief Types of binary operators acting on where clause subexpressions */
enum UnaryLogicalOperatorType
{
    eLogicalOperatorNot,
};

/** \brief Types of binary operators used for combination of two ExpressionWhereClause objects */
enum ConditionalOperatorType
{
    eConditionalOperatorAnd,
    eConditionalOperatorOr
};

namespace detail
{

class ExpressionNodeImplBase;

class ExpressionWhereClauseImplBase : public std::enable_shared_from_this<ExpressionWhereClauseImplBase>
{
public:
    ExpressionWhereClauseImplBase();
    virtual ~ExpressionWhereClauseImplBase();

    virtual String sqlExpression() const = 0;
    virtual bool complex() const = 0;
    virtual WhereClauseExpressionType type() const = 0;
};

typedef std::shared_ptr<ExpressionWhereClauseImplBase> ExpressionWhereClauseImplPtr;

class ExpressionWhereClauseImplRelational : public ExpressionWhereClauseImplBase
{
public:
     ExpressionWhereClauseImplRelational(RelationOperatorType op, const ExpressionNodeBase& lhs, const ExpressionNodeBase& rhs);
     ExpressionWhereClauseImplRelational(const ExpressionWhereClauseImplRelational&)=default;
    ~ExpressionWhereClauseImplRelational();

    String sqlExpression() const override;
    bool complex() const override;
    WhereClauseExpressionType type() const;

    RelationOperatorType operatorType() const;
private:
    RelationOperatorType m_operator;
    std::shared_ptr<detail::ExpressionNodeImplBase> m_lhs, m_rhs;
};

class ExpressionWhereClauseImplLogical : public ExpressionWhereClauseImplBase
{
public:
    ExpressionWhereClauseImplLogical(UnaryLogicalOperatorType op, const ExpressionWhereClause& inner);
    ~ExpressionWhereClauseImplLogical();

    String sqlExpression() const override;
    bool complex() const override;
    WhereClauseExpressionType type() const;

    UnaryLogicalOperatorType operatorType() const;
private:
    UnaryLogicalOperatorType m_operator;
    ExpressionWhereClauseImplPtr m_inner;
};

class ExpressionWhereClauseImplComplex : public ExpressionWhereClauseImplBase
{
public:
    ExpressionWhereClauseImplComplex(ConditionalOperatorType op, const ExpressionWhereClauseImplPtr& lhs, const ExpressionWhereClauseImplPtr& rhs);
    ~ExpressionWhereClauseImplComplex();

    String sqlExpression() const override;
    bool complex() const override;
    WhereClauseExpressionType type() const;

    ConditionalOperatorType operatorType() const;
private:
    ConditionalOperatorType m_operator;
    ExpressionWhereClauseImplPtr m_lhs, m_rhs;
};

}

/** \brief Class representing final conditional expression which usually appears in where clause part of an SQL statement */
class ExpressionWhereClause final
{
public:
    /** \brief Constructs new instance of ExpressionWhereClause with given private implementation */
    explicit ExpressionWhereClause(const detail::ExpressionWhereClauseImplPtr& impl);
    virtual ~ExpressionWhereClause();

    /** \brief Gets the private implementation */
    detail::ExpressionWhereClauseImplPtr impl() const;
private:
    detail::ExpressionWhereClauseImplPtr m_impl;
};

ExpressionWhereClause operator&&(const ExpressionWhereClause& lhs, const ExpressionWhereClause& rhs);
ExpressionWhereClause operator||(const ExpressionWhereClause& lhs, const ExpressionWhereClause& rhs);
ExpressionWhereClause operator!(const ExpressionWhereClause& inner);

} // namespace db
} // namespace metacpp

#endif // METACPP_DB_EXPRESSIONWHERECLAUSE_H
