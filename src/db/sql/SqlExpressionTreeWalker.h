#ifndef SQLEXPRESSIONTREEWALKER_H
#define SQLEXPRESSIONTREEWALKER_H
#include "ASTWalkerBase.h"

namespace metacpp
{
namespace db
{
namespace sql
{

/** \brief Type of the sql syntax used by SqlStatementBase
 * \relates SqlStatementBase */
enum SqlSyntax
{
    SqlSyntaxUnknown,       /**< Invalid syntax type */
    SqlSyntaxSqlite,        /**< Sqlite syntax */
    SqlSyntaxPostgreSQL,    /**< Postgresql syntax */
    SqlSyntaxMysql,         /**< Mysql syntax */
    SqlSyntaxMssql,         /**< Microsoft SQL Server syntax */
    SqlSyntaxFirebird,      /**< Firebird/Interbase syntax */
    SqlSyntaxOracle         /**< Oracle RDBMS syntax */
};

namespace detail
{

class SqlExpressionTreeWalker : public db::detail::ASTWalkerBase
{
public:
    explicit SqlExpressionTreeWalker(const db::detail::ExpressionNodeImplPtr& rootNode, bool fullQualified = true, SqlSyntax sqlSyntax = SqlSyntaxUnknown);
    ~SqlExpressionTreeWalker();

protected:
    String evaluateColumn(std::shared_ptr<db::detail::ExpressionNodeImplColumn> column) override;
    String evaluateLiteral(std::shared_ptr<db::detail::ExpressionNodeImplLiteral> literal) override;
    String evaluateNull(std::shared_ptr<db::detail::ExpressionNodeImplNull> null) override;
    String evaluateUnaryOperator(std::shared_ptr<db::detail::ExpressionNodeImplUnaryOperator> unary) override;
    String evaluateBinaryOperator(std::shared_ptr<db::detail::ExpressionNodeImplBinaryOperator> binary) override;
    String evaluateFunctionCall(std::shared_ptr<db::detail::ExpressionNodeImplFunctionCall> functionCall) override;
    String evaluateWhereClauseRelational(std::shared_ptr<db::detail::ExpressionNodeImplWhereClauseRelational> whereClauseRelational) override;
    String evaluateWhereClauseLogical(std::shared_ptr<db::detail::ExpressionNodeImplWhereClauseLogical> whereClauseLogical) override;
    String evaluateWhereClauseConditional(std::shared_ptr<db::detail::ExpressionNodeImplWhereClauseConditional> whereClauseConditional) override;
private:
    String evaluateSubnode(const db::detail::ExpressionNodeImplPtr& node, bool bracesRequired);
private:
    bool m_fullQualified;
    SqlSyntax m_sqlSyntax;
};

} // namespace detail
} // namespace sql
} // namespace db
} // namespace metacpp

#endif // SQLEXPRESSIONTREEWALKER_H

