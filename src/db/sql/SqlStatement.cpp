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
#include "SqlStatementImpl.h"
#include "SqlStatement.h"
#include "SqlTransaction.h"
#include "SqlStorable.h"

namespace metacpp
{
namespace db
{
namespace sql
{

static String quote(const char *name, SqlSyntax syntax)
{
    static String quote_char = "\"";
    static String mysql_quote_char = "`";
    switch (syntax)
    {
    case SqlSyntaxMySql:
        return mysql_quote_char + name + mysql_quote_char;
    default:
        return quote_char + name + quote_char;
    }
}

SqlStatementBase::SqlStatementBase()
{

}

SqlStatementBase::~SqlStatementBase()
{

}

SharedObjectPointer<connectors::SqlStatementImpl> SqlStatementBase::createImpl(SqlTransaction& transaction)
{
    auto transactionImpl = transaction.impl();
    connectors::SqlStatementImpl *stmt = transactionImpl->createStatement(type(), buildQuery(transaction.connector()->sqlSyntax()));
    if (!stmt)
        throw std::runtime_error("Failed to create statement");
    SharedObjectPointer<connectors::SqlStatementImpl> impl(stmt,
        [transactionImpl](connectors::SqlStatementImpl *stmt){ transactionImpl->closeStatement(stmt); });
    return m_impl = impl;
}

SqlStatementSelect::SqlStatementSelect(SqlStorable *storable)
    : m_joinType(JoinTypeNone), m_storable(storable)
{

}

SqlStatementSelect::~SqlStatementSelect()
{

}

SqlStatementType SqlStatementSelect::type() const
{
    return SqlStatementTypeSelect;
}

String SqlStatementSelect::buildQuery(SqlSyntax syntax)
{
    (void)syntax;
    if (!m_storable->record()->metaObject()->totalFields())
        throw std::runtime_error("Invalid storable");
    String res;
    const char *tblName = m_storable->record()->metaObject()->name();
    StringArray columns;
    for (size_t i = 0; i < m_storable->record()->metaObject()->totalFields(); ++i)
        columns.push_back(quote(tblName, syntax) + "." +
                          quote(m_storable->record()->metaObject()->field(i)->name(), syntax));
    res = "SELECT " + join(columns, ", ") + " FROM " + quote(tblName, syntax);
    if (!m_whereClause.empty())
    {
        detail::SqlExpressionTreeWalker walker(m_whereClause.impl(), true, syntax);
        String whereExpr = walker.evaluate();
        m_literals = walker.literals();
        if (m_joins.size())
        {
            switch (m_joinType)
            {
            case JoinTypeInner:
                res += " INNER JOIN ";
                break;
            case JoinTypeLeftOuter:
                res += " LEFT OUTER JOIN ";
                break;
            default:
                throw std::runtime_error("Unknown join type");
            }

            for (size_t i = 0; i < m_joins.size(); ++i)
            {
                res += quote(m_joins[i]->name(), syntax);
                if (m_joins.size() - 1 != i) res += ", ";
            }
            res += " ON " + whereExpr;
        }
        else
        {
            res += " WHERE " + whereExpr;
        }
    }
    if (m_order.size())
    {
        auto orders = m_order.template map<String>(
            [=](const std::pair<db::detail::ExpressionNodeImplPtr, bool>& order)
                    -> String
            {
                return detail::SqlExpressionTreeWalker(order.first, true, syntax)
                        .evaluate() + (order.second ? " ASC" : "DESC");
            });
        res += " ORDER BY " + join(orders, ", ");
    }
    if (m_limit) res += " LIMIT " + String::fromValue(*m_limit);
    if (m_offset) res += " OFFSET " + String::fromValue(*m_offset);
    return res;
}

SqlStatementSelect &SqlStatementSelect::limit(size_t lim)
{
    m_limit = lim;
    return *this;
}

SqlStatementSelect &SqlStatementSelect::offset(size_t off)
{
    m_offset = off;
    return *this;
}

SqlStatementSelect &SqlStatementSelect::where(const ExpressionNodeWhereClause &whereClause)
{
    m_whereClause = whereClause;
    return *this;
}

SqlResultSet SqlStatementSelect::exec(SqlTransaction &transaction)
{
    SqlResultSet res(transaction, createImpl(transaction), m_storable);
    if (!transaction.impl()->prepare(m_impl.get(), m_literals.size()))
        throw std::runtime_error("Failed to prepare statement");
    if (m_literals.size() && !transaction.impl()->bindValues(m_impl.get(), m_literals))
        throw std::runtime_error("Failed to bind values");
    return res;
}

bool SqlStatementSelect::fetchOne(SqlTransaction &transaction)
{
    auto result = exec(transaction);
    return result.begin() != result.end();
}

SqlStatementInsert::SqlStatementInsert(SqlStorable *storable)
    : m_storable(storable), m_numLiterals(0), m_prepared(false)
{
}

SqlStatementInsert::~SqlStatementInsert()
{
}

SqlStatementType SqlStatementInsert::type() const
{
    return SqlStatementTypeInsert;
}

String SqlStatementInsert::buildQuery(SqlSyntax syntax)
{
    (void)syntax;
    if (!m_storable->record()->metaObject()->totalFields())
        throw std::runtime_error("Invalid storable");
    String res;
    const char *tblName = m_storable->record()->metaObject()->name();
    res = "INSERT INTO " + quote(tblName, syntax);
    auto pkey = m_storable->primaryKey();
    StringArray columns, placeholders;
    for (size_t i = 0; i < m_storable->record()->metaObject()->totalFields(); ++i)
    {
        auto field = m_storable->record()->metaObject()->field(i);
        if (field != pkey)
        {
            columns.push_back(quote(field->name(), syntax));
            if (SqlSyntaxPostgreSQL == syntax)
                placeholders.push_back("$" + String::fromValue(placeholders.size() + 1));
            else
                placeholders.push_back("?");
        }
    }
    res += "(" + join(columns, ", ") + ") VALUES (" + join(placeholders, ", ") + ")";
    assert(columns.size() == placeholders.size());
    m_numLiterals = columns.size();
    return res;
}

int SqlStatementInsert::execPrepare(SqlTransaction &transaction)
{
    if (m_prepared)
        throw std::logic_error("Statement is already prepared");
    createImpl(transaction);
    if (!transaction.impl()->prepare(m_impl.get(), m_numLiterals))
        throw std::runtime_error("Failed to prepare statement");
    m_prepared = true;

    return 0;
}

int SqlStatementInsert::execStep(SqlTransaction &transaction, const Object *record)
{
    if (!m_prepared)
        throw std::logic_error("Statement must be prepared first");
    if (m_storable->record()->metaObject() != record->metaObject())
        throw std::invalid_argument("Cannot mix storable types in insert request");

    m_literals.clear();
    m_literals.reserve(m_numLiterals);
    auto pkey = m_storable->primaryKey();
    for (size_t i = 0; i < record->metaObject()->totalFields(); ++i)
    {
        auto field = record->metaObject()->field(i);
        if (field != pkey)
        {
            m_literals.push_back(field->getValue(record));
        }
    }
    if (m_literals.size() != m_numLiterals)
        throw std::logic_error("Unexpected number of columns in record");

    if (m_literals.size() && !transaction.impl()->bindValues(m_impl.get(), m_literals))
        throw std::runtime_error("Failed to bind values");
    int numRows = 0;
    if (!transaction.impl()->execStatement(m_impl.get(), &numRows))
        throw std::runtime_error("Failed to execute statement");
    transaction.impl()->getLastInsertId(m_impl.get(), m_storable);
    return numRows;
}

SqlStatementUpdate::SqlStatementUpdate(SqlStorable *storable)
    : m_storable(storable)
{
}

SqlStatementUpdate::~SqlStatementUpdate()
{
}

SqlStatementType SqlStatementUpdate::type() const
{
    return SqlStatementTypeUpdate;
}

String SqlStatementUpdate::buildQuery(SqlSyntax syntax)
{
    if (!m_storable->record()->metaObject()->totalFields())
        throw std::runtime_error("Invalid storable");
    String res;
    const char *tblName = m_storable->record()->metaObject()->name();
    res = "UPDATE " + quote(tblName, syntax);
    auto pkey = m_storable->primaryKey();
    StringArray sets;
    if (!m_sets.size())
    {
        m_literals.clear();
        for (size_t i = 0; i < m_storable->record()->metaObject()->totalFields(); ++i)
        {
            auto field = m_storable->record()->metaObject()->field(i);
            if (field != pkey)
            {
                m_literals.push_back(field->getValue(m_storable->record()));
                if (syntax == SqlSyntaxPostgreSQL)
                    sets.push_back(quote(field->name(), syntax) + " = $" +
                                   String::fromValue(m_literals.size()));
                else
                    sets.push_back(quote(field->name(), syntax) + " = ?");
            }
        }
    }
    else {
        sets.reserve(m_sets.size());
        m_literals.reserve(m_sets.size());
        for (auto& set : m_sets)
        {
            String expr = detail::SqlExpressionTreeWalker(set.first, false, syntax)
                    .evaluate() + " = ";
            detail::SqlExpressionTreeWalker walker(set.second, true, syntax, m_literals.size());
            expr += walker.evaluate();
            m_literals.append(walker.literals());
            sets.push_back(expr);
        }
    }

    String whereExpr;
    if (!m_whereClause.empty())
    {
        detail::SqlExpressionTreeWalker walker(m_whereClause.impl(), true, syntax, m_literals.size());
        whereExpr = walker.evaluate();
        m_literals.append(walker.literals());
    }

    if (m_joins.size())
    {
        String joins;
        for (size_t i = 0; i < m_joins.size(); ++i)
        {
            joins += quote(m_joins[i]->name(), syntax);
            if (i != m_joins.size() - 1)
                joins += ", ";
        }

        if (SqlSyntaxSqlite == syntax || SqlSyntaxMySql == syntax)
        {
            res += " SET " + join(sets, ", ") +
                   " WHERE EXISTS (SELECT 1 FROM " + joins;

            if (!m_whereClause.empty()) res += " WHERE " + whereExpr + ")";
        }
        else if (SqlSyntaxPostgreSQL == syntax)
        {
            res += " SET " + join(sets, ", ") +
                   " FROM " + joins;
            if (!m_whereClause.empty()) res += " WHERE " + whereExpr;
        }
        else
            throw std::runtime_error("Unimplemented syntax");
    }
    else
    {
        res += " SET " + join(sets, ", ");
        if (!whereExpr.isNullOrEmpty()) res += " WHERE " + whereExpr;
    }
    return res;
}

SqlStatementUpdate &SqlStatementUpdate::where(const ExpressionNodeWhereClause &whereClause)
{
    m_whereClause = whereClause;
    return *this;
}

int SqlStatementUpdate::exec(SqlTransaction &transaction)
{
    createImpl(transaction);
    if (!transaction.impl()->prepare(m_impl.get(), m_literals.size()))
        throw std::runtime_error("Failed to prepare statement");
    if (m_literals.size() && !transaction.impl()->bindValues(m_impl.get(), m_literals))
        throw std::runtime_error("Failed to bind values");
    int numRows = 0;
    if (!transaction.impl()->execStatement(m_impl.get(), &numRows))
        throw std::runtime_error("Failed to execute statement");
    return numRows;
}

SqlStatementDelete::SqlStatementDelete(SqlStorable *storable)
    : m_storable(storable)
{
}

SqlStatementDelete::~SqlStatementDelete()
{
}

SqlStatementType SqlStatementDelete::type() const
{
    return SqlStatementTypeDelete;
}

String SqlStatementDelete::buildQuery(SqlSyntax syntax)
{
    String res;
    const char *tblName = m_storable->record()->metaObject()->name();

    String whereExpr;
    if (!m_whereClause.empty())
    {
        detail::SqlExpressionTreeWalker walker(m_whereClause.impl(), true, syntax);
        whereExpr = walker.evaluate();
        m_literals.append(walker.literals());
    }

    if (m_joins.size())
    {
        StringArray joins = m_joins.template map<String>(
                    [=](const MetaObject * mo) { return quote(mo->name(), syntax); });
        if (SqlSyntaxSqlite == syntax)
        {
            res = "DELETE FROM " + quote(tblName, syntax) +
                    " WHERE EXISTS (SELECT 1 FROM " + join(joins, ", ");
            if (!whereExpr.isNullOrEmpty()) res += " WHERE " + whereExpr;
            res += ")";
        }
        else if (SqlSyntaxPostgreSQL == syntax)
        {
            res = "DELETE FROM " + quote(tblName, syntax) + " USING " + join(joins, ", ");
            if (!whereExpr.isNullOrEmpty()) res += " WHERE " + whereExpr;
        }
        else // MySql
        {
            res = "DELETE " + quote(tblName, syntax) + " FROM " + quote(tblName, syntax) +
                    " JOIN " + join(joins, " JOIN ");
            if (!whereExpr.isNullOrEmpty()) res += " WHERE " + whereExpr;
        }
    }
    else
    {
        res = "DELETE FROM " + quote(tblName, syntax);
        if (!whereExpr.isNullOrEmpty()) res += " WHERE " + whereExpr;
    }
    return res;

}

SqlStatementDelete &SqlStatementDelete::where(const ExpressionNodeWhereClause &whereClause)
{
    m_whereClause = whereClause;
    return *this;
}

int SqlStatementDelete::exec(SqlTransaction &transaction)
{
    createImpl(transaction);
    if (!transaction.impl()->prepare(m_impl.get(), m_literals.size()))
        throw std::runtime_error("Failed to prepare statement");
    if (m_literals.size() && !transaction.impl()->bindValues(m_impl.get(), m_literals))
        throw std::runtime_error("Failed to bind values");
    int numRows = 0;
    if (!transaction.impl()->execStatement(m_impl.get(), &numRows))
        throw std::runtime_error("Failed to execute statement");
    return numRows;
}

SqlStatementCustom::SqlStatementCustom(const String &queryText)
    : m_queryText(queryText)
{

}

SqlStatementCustom::~SqlStatementCustom()
{
}

SqlStatementType SqlStatementCustom::type() const
{
    return SqlStatementTypeUnknown;
}

String SqlStatementCustom::buildQuery(SqlSyntax syntax)
{
    (void)syntax;
    return m_queryText;
}

void SqlStatementCustom::exec(SqlTransaction &transaction)
{
    createImpl(transaction);
    if (!transaction.impl()->prepare(m_impl.get(), m_literals.size()))
        throw std::runtime_error("Failed to prepare statement");
    if (m_literals.size() && !transaction.impl()->bindValues(m_impl.get(), m_literals))
        throw std::runtime_error("Failed to bind values");
    if (!transaction.impl()->execStatement(m_impl.get()))
        throw std::runtime_error("Failed to execute statement");
}

} // namespace sql
} // namespace db
} // namespace metacpp
