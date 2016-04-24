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
#ifndef SQLEXPRESSIONTREEWALKER_H
#define SQLEXPRESSIONTREEWALKER_H
#include "config.h"
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
    SqlSyntaxMySql,         /**< Mysql syntax */
    SqlSyntaxMssql,         /**< Microsoft SQL Server syntax */
    SqlSyntaxFirebird,      /**< Firebird/Interbase syntax */
    SqlSyntaxOracle         /**< Oracle RDBMS syntax */
};

namespace detail
{

class SqlExpressionTreeWalker final : db::detail::ASTWalkerBase
{
public:
    explicit SqlExpressionTreeWalker(const db::detail::ExpressionNodeImplPtr& rootNode, 
		bool fullQualified = true, SqlSyntax sqlSyntax = SqlSyntaxUnknown, size_t startLiteralIndex = 0);
    ~SqlExpressionTreeWalker();

    String evaluate();
    const VariantArray& literals() const;

protected:
    void visitColumn(std::shared_ptr<db::detail::ExpressionNodeImplColumn> column) override;
    void visitLiteral(std::shared_ptr<db::detail::ExpressionNodeImplLiteral> literal) override;
    void visitNull(std::shared_ptr<db::detail::ExpressionNodeImplNull> null) override;
    void visitUnaryOperator(std::shared_ptr<db::detail::ExpressionNodeImplUnaryOperator> unary) override;
    void visitCastOperator(std::shared_ptr<db::detail::ExpressionNodeImplCastOperator> cast) override;
    void visitBinaryOperator(std::shared_ptr<db::detail::ExpressionNodeImplBinaryOperator> binary) override;
    void visitFunctionCall(std::shared_ptr<db::detail::ExpressionNodeImplFunctionCall> functionCall) override;
    void visitWhereClauseRelational(std::shared_ptr<db::detail::ExpressionNodeImplWhereClauseRelational> whereClauseRelational) override;
    void visitWhereClauseLogical(std::shared_ptr<db::detail::ExpressionNodeImplWhereClauseLogical> whereClauseLogical) override;
    void visitWhereClauseConditional(std::shared_ptr<db::detail::ExpressionNodeImplWhereClauseConditional> whereClauseConditional) override;
private:
    String evaluateSubnode(const db::detail::ExpressionNodeImplPtr& node, bool bracesRequired = false);
private:
    bool m_fullQualified;
    SqlSyntax m_sqlSyntax;
    Array<String> m_stack;
    VariantArray m_bindValues;
    size_t m_startLiteralIndex;
};

} // namespace detail
} // namespace sql
} // namespace db
} // namespace metacpp

#endif // SQLEXPRESSIONTREEWALKER_H

