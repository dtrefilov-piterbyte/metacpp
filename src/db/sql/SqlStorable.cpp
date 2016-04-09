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
#include "SqlStorable.h"
#include "Nullable.h"
#include "SqlTransaction.h"

namespace metacpp
{
namespace db
{
namespace sql
{

SqlStorable::SqlStorable()
{
}

SqlStorable::~SqlStorable()
{
}

SqlStatementSelect SqlStorable::select()
{
    return SqlStatementSelect(this);
}

SqlStatementDelete SqlStorable::remove()
{
    return SqlStatementDelete(this);
}

SqlStatementUpdate SqlStorable::update()
{
    return SqlStatementUpdate(this);
}

bool SqlStorable::insertOne(SqlTransaction& transaction)
{
    SqlStatementInsert statement(this);
    statement.execPrepare(transaction);
    int nRows = statement.execStep(transaction, record());
    return nRows < 0 || nRows == 1;
}

bool SqlStorable::updateOne(SqlTransaction &transaction)
{
    SqlStatementUpdate statement(this);
    int nRows = statement.where(whereId()).exec(transaction);
    return nRows < 0 || nRows == 1;
}

bool SqlStorable::removeOne(SqlTransaction &transaction)
{
    SqlStatementDelete statement(this);
    int nRows = statement.where(whereId()).exec(transaction);
    return nRows < 0 || nRows == 1;
}

template<typename T>
static String GetArithmeticFieldValue(const MetaFieldBase *field, Object *obj)
{
    if (field->nullable())
    {
        auto value = field->access<Nullable<T> >(obj);
        if (!value)
            return "NULL";
        else
            return String::fromValue(*value);
    }
    return String::fromValue(field->access<T>(obj));
}

String SqlStorable::fieldValue(const MetaFieldBase *field) const
{
    switch(field->type())
    {
    case eFieldBool:
        return GetArithmeticFieldValue<bool>(field, const_cast<SqlStorable *>(this)->record());
    case eFieldInt:
        return GetArithmeticFieldValue<int32_t>(field, const_cast<SqlStorable *>(this)->record());
    case eFieldUint:
    case eFieldEnum:
        return GetArithmeticFieldValue<uint32_t>(field, const_cast<SqlStorable *>(this)->record());
    case eFieldInt64:
        return GetArithmeticFieldValue<int64_t>(field, const_cast<SqlStorable *>(this)->record());
    case eFieldUint64:
        return GetArithmeticFieldValue<uint64_t>(field, const_cast<SqlStorable *>(this)->record());
    case eFieldFloat:
        return GetArithmeticFieldValue<float>(field, const_cast<SqlStorable *>(this)->record());
    case eFieldDouble:
        return GetArithmeticFieldValue<double>(field, const_cast<SqlStorable *>(this)->record());
    case eFieldString: {
        if (field->nullable())
        {
            if (!field->access<Nullable<String> >(const_cast<SqlStorable *>(this)->record()))
                return "NULL";
            else
            {
                String val = *field->access<Nullable<String> >(const_cast<SqlStorable *>(this)->record());
                return "\'" + val.replace("'", "''") + "\'";
            }
        }
        String val = field->access<String>(const_cast<SqlStorable *>(this)->record());
        return "\'" + val.replace("'", "''") + "\'";
    }
    case eFieldObject:
    case eFieldVariant:
    case eFieldArray:
        throw std::runtime_error("Can store only plain objects");
    case eFieldDateTime:
        if (field->nullable())
        {
            if (!field->access<Nullable<DateTime> >(const_cast<SqlStorable *>(this)->record()))
                return "NULL";
            else
                return "\'" + field->access<Nullable<DateTime> >(const_cast<SqlStorable *>(this)->record()).get().toString() + "\'";
        }
        return "\'" + field->access<DateTime>(const_cast<SqlStorable *>(this)->record()).toString() + "\'";
    default:
        throw std::runtime_error("Unknown field type");
    }
}

void SqlStorable::createSchema(SqlTransaction &transaction, const MetaObject *metaObject,
                               const Array<SqlConstraintBasePtr> &constraints)
{
    SqlSyntax syntax = transaction.connector()->sqlSyntax();
    switch (syntax)
    {
    case SqlSyntaxSqlite:
        return createSchemaSqlite(transaction, metaObject, constraints);
    case SqlSyntaxPostgreSQL:
        return createSchemaPostgreSQL(transaction, metaObject, constraints);
    case SqlSyntaxMySql:
        return createSchemaMySql(transaction, metaObject, constraints);
    default:
        throw std::runtime_error("SqlStorable::createSchema(): syntax not implemented");
    }
}

ExpressionNodeWhereClause SqlStorable::whereId()
{
    auto pkey = primaryKey();
    if (!pkey)
        throw std::runtime_error(std::string("Table ") + record()->metaObject()->name() +
                                 " has no primary key");
    return ExpressionNodeWhereClause(std::make_shared<::metacpp::db::detail::ExpressionNodeImplWhereClauseRelational>(eRelationalOperatorEqual,
        std::make_shared<db::detail::ExpressionNodeImplColumn>(pkey),
        std::make_shared<db::detail::ExpressionNodeImplLiteral>(pkey->getValue(record()))));
}

void SqlStorable::createSchemaSqlite(SqlTransaction &transaction, const MetaObject *metaObject, const Array<SqlConstraintBasePtr> &constraints)
{
    String tblName = metaObject->name();
    // validate constraints
    for (SqlConstraintBasePtr constraint : constraints)
    {
        if (constraint->metaObject() != metaObject)
            throw std::runtime_error("Constraint does not belong to this table");
    }

    String queryStr = "CREATE TABLE IF NOT EXISTS " + tblName + "(";
    StringArray columns;
    auto findConstraint = [constraints](SqlConstraintType type, const MetaFieldBase *field)
    {
        for (SqlConstraintBasePtr constraint : constraints)
        {
            if (constraint->type() == type && field == constraint->metaField())
                return constraint;
        }
        return SqlConstraintBasePtr();
    };

    for (size_t i = 0; i < metaObject->totalFields(); ++i)
    {
        const MetaFieldBase *field = metaObject->field(i);
        String name = field->name();
        String typeName;
        StringArray constraints;
        if (!field->nullable())
            constraints.push_back("NOT NULL");
        switch (field->type())
        {
        case eFieldBool:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldBool *>(field)->defaultValue()), false, SqlSyntaxSqlite).evaluate());
            }
            break;
        case eFieldInt:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldInt *>(field)->defaultValue()), false, SqlSyntaxSqlite).evaluate());
            }
            break;
        case eFieldEnum:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldEnum *>(field)->defaultValue()), false, SqlSyntaxSqlite).evaluate());
            }
            break;
        case eFieldUint:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldUint *>(field)->defaultValue()), false, SqlSyntaxSqlite).evaluate());
            }
            break;
        case eFieldInt64:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldInt64 *>(field)->defaultValue()), false, SqlSyntaxSqlite).evaluate());
            }
            break;
        case eFieldUint64:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldUint64 *>(field)->defaultValue()), false, SqlSyntaxSqlite).evaluate());
            }
            break;
        case eFieldFloat:
            typeName = "REAL";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldFloat *>(field)->defaultValue()), false, SqlSyntaxSqlite).evaluate());
            }
            break;
        case eFieldDouble:
            typeName = "REAL";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldDouble *>(field)->defaultValue()), false, SqlSyntaxSqlite).evaluate());
            }
            break;
        case eFieldString:
            typeName = "TEXT";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldString *>(field)->defaultValue()), false, SqlSyntaxSqlite).evaluate());
            }
            break;
        case eFieldDateTime:
            typeName = "DATETIME";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldDateTime *>(field)->defaultValue()), false, SqlSyntaxSqlite).evaluate());
            }
            break;
        default:
            throw std::runtime_error(std::string("Cannot handle field ") + field->name() + " as an sql column");
        }

        auto primaryKey = findConstraint(SqlConstraintTypePrimaryKey, field);
        if (primaryKey)
        {
            constraints.push_back("PRIMARY KEY AUTOINCREMENT");
        }

        auto foreignKey = std::dynamic_pointer_cast<SqlConstraintForeignKey>(
                    findConstraint(SqlConstraintTypeForeignKey, field));
        if (foreignKey)
        {
            constraints.push_back(String("REFERENCES ") + foreignKey->referenceMetaObject()->name() +
                                  "(" + foreignKey->referenceMetaField()->name() + ")");
        }

        auto check = findConstraint(SqlConstraintTypeCheck, field);
        if (check)
        {
            constraints.push_back("CHECK (" +
                                  std::dynamic_pointer_cast<SqlConstraintCheck>(check)->checkExpression()
                                  + ")");
        }

        String column = name + " " + typeName + (constraints.size() ? " " : "") + join(constraints, " ");
        columns.push_back(column);
    }
    queryStr += join(columns, ", ") + ")";

    // main statement CREATE TABLE
    SqlStatementCustom statement(queryStr);
    statement.exec(transaction);

    for (auto constraint : constraints)
    {
        if (constraint->type() == SqlConstraintTypeIndex)
        {
            std::shared_ptr<SqlConstraintIndex> constrIdx = std::dynamic_pointer_cast<SqlConstraintIndex>(constraint);
            String queryStr = constrIdx->unique() ?
                        "CREATE UNIQUE INDEX IF NOT EXISTS" : "CREATE INDEX IF NOT EXISTS";
            queryStr += " idx_" + tblName + "_" + constrIdx->metaField()->name() +
                    " ON " + tblName + "(" + constrIdx->metaField()->name() + ")";
            SqlStatementCustom statement(queryStr);
            statement.exec(transaction);
        }
    }
}

void SqlStorable::createSchemaPostgreSQL(SqlTransaction &transaction, const MetaObject *metaObject,
                                         const Array<SqlConstraintBasePtr> &constraints)
{
    String tblName = metaObject->name();
    // validate constraints
    for (SqlConstraintBasePtr constraint : constraints)
    {
        if (constraint->metaObject() != metaObject)
            throw std::runtime_error("Constraint does not belong to this table");
    }

    String queryStr = "CREATE TABLE IF NOT EXISTS " + tblName + "(";
    StringArray columns;
    auto findConstraint = [constraints](SqlConstraintType type, const MetaFieldBase *field)
    {
        for (SqlConstraintBasePtr constraint : constraints)
        {
            if (constraint->type() == type && field == constraint->metaField())
                return constraint;
        }
        return SqlConstraintBasePtr();
    };

    for (size_t i = 0; i < metaObject->totalFields(); ++i)
    {
        const MetaFieldBase *field = metaObject->field(i);
        String name = field->name();
        String typeName;
        StringArray constraints;
        if (!field->nullable())
            constraints.push_back("NOT NULL");
        switch (field->type())
        {
        case eFieldBool:
            typeName = "SMALLINT";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldBool *>(field)->defaultValue()), false, SqlSyntaxPostgreSQL).evaluate());
            }
            break;
        case eFieldInt:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldInt *>(field)->defaultValue()), false, SqlSyntaxPostgreSQL).evaluate());
            }
            break;
        case eFieldEnum:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldEnum *>(field)->defaultValue()), false, SqlSyntaxPostgreSQL).evaluate());
            }
            break;
        case eFieldUint:
            typeName = "BIGINT";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldUint *>(field)->defaultValue()), false, SqlSyntaxPostgreSQL).evaluate());
            }
            break;
        case eFieldInt64:
            typeName = "BIGINT";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldInt64 *>(field)->defaultValue()), false, SqlSyntaxPostgreSQL).evaluate());
            }
            break;
        case eFieldUint64:
            typeName = "BIGINT";    // NUMERIC?
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldUint64 *>(field)->defaultValue()), false, SqlSyntaxPostgreSQL).evaluate());
            }
            break;
        case eFieldFloat:
            typeName = "REAL";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldFloat *>(field)->defaultValue()), false, SqlSyntaxPostgreSQL).evaluate());
            }
            break;
        case eFieldDouble:
            typeName = "DOUBLE PRECISION";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldDouble *>(field)->defaultValue()), false, SqlSyntaxPostgreSQL).evaluate());
            }
            break;
        case eFieldString:
            typeName = "TEXT";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldString *>(field)->defaultValue()), false, SqlSyntaxPostgreSQL).evaluate());
            }
            break;
        case eFieldDateTime:
            typeName = "TIMESTAMP";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                    (reinterpret_cast<const MetaFieldDateTime *>(field)->defaultValue()), false, SqlSyntaxPostgreSQL).evaluate());
            }
            break;
        default:
            throw std::runtime_error(std::string("Cannot handle field ") + field->name() + " as an sql column");
        }

        auto primaryKey = findConstraint(SqlConstraintTypePrimaryKey, field);
        if (primaryKey)
        {
            if (typeName == "INTEGER")
                typeName = "SERIAL";
            else if (typeName == "BIGINT")
                typeName = "BIGSERIAL";
            else
                throw std::runtime_error("Primary keys are only allowed on integral types");
        }

        auto foreignKey = std::dynamic_pointer_cast<SqlConstraintForeignKey>(
                    findConstraint(SqlConstraintTypeForeignKey, field));
        if (foreignKey)
        {
            constraints.push_back(String("REFERENCES ") + foreignKey->referenceMetaObject()->name() +
                                  "(" + foreignKey->referenceMetaField()->name() + ")");
        }

        auto check = findConstraint(SqlConstraintTypeCheck, field);
        if (check)
        {
            constraints.push_back("CHECK (" +
                                  std::dynamic_pointer_cast<SqlConstraintCheck>(check)->checkExpression()
                                  + ")");
        }

        String column = name + " " + typeName + (constraints.size() ? " " : "") + join(constraints, " ");
        columns.push_back(column);
    }
    queryStr += join(columns, ", ") + ")";

    // main statement CREATE TABLE
    SqlStatementCustom statement(queryStr);
    statement.exec(transaction);

    for (auto constraint : constraints)
    {
        if (constraint->type() == SqlConstraintTypeIndex)
        {
            std::shared_ptr<SqlConstraintIndex> constrIdx = std::dynamic_pointer_cast<SqlConstraintIndex>(constraint);
            String columnName = constrIdx->metaField()->name();
            String indexName = "idx_" + tblName + "_" + columnName;
            String queryStr =
                    "DO $$ DECLARE index_exists boolean;"
                    "BEGIN "
                    "index_exists := FALSE;"
                    "SELECT TRUE INTO index_exists FROM pg_indexes WHERE indexname = lower(\'" + indexName + "\') LIMIT 1;"
                    "IF NOT FOUND THEN" +
                    (constrIdx->unique() ? " CREATE UNIQUE INDEX " : " CREATE INDEX ") + indexName +
                    " ON " + tblName + "(" + columnName + ");"
                    "END IF;"
                    "END$$ LANGUAGE plpgsql";
            SqlStatementCustom statement(queryStr);
            statement.exec(transaction);
        }
    }
}

void SqlStorable::createSchemaMySql(SqlTransaction &transaction, const MetaObject *metaObject, const Array<SqlConstraintBasePtr> &constraints)
{
    String tblName = metaObject->name();
    // validate constraints
    for (SqlConstraintBasePtr constraint : constraints)
    {
        if (constraint->metaObject() != metaObject)
            throw std::runtime_error("Constraint does not belong to this table");
    }

    String queryStr = "CREATE TABLE " + tblName + "(";
    StringArray columns;
    auto findConstraint = [constraints](SqlConstraintType type, const MetaFieldBase *field)
    {
        for (SqlConstraintBasePtr constraint : constraints)
        {
            if (constraint->type() == type && field == constraint->metaField())
                return constraint;
        }
        return SqlConstraintBasePtr();
    };

    for (size_t i = 0; i < metaObject->totalFields(); ++i)
    {
        const MetaFieldBase *field = metaObject->field(i);
        String name = field->name();
        String typeName;
        StringArray constraints;
        if (!field->nullable())
            constraints.push_back("NOT NULL");
        switch (field->type())
        {
        case eFieldBool:
            typeName = "TINYINT";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                                                                                   (reinterpret_cast<const MetaFieldBool *>(field)->defaultValue()), false, SqlSyntaxMySql).evaluate());
            }
            break;
        case eFieldInt:
            typeName = "INT";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                                                                                   (reinterpret_cast<const MetaFieldInt *>(field)->defaultValue()), false, SqlSyntaxMySql).evaluate());
            }
            break;
        case eFieldEnum:
            typeName = "INT";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                                                                                   (reinterpret_cast<const MetaFieldEnum *>(field)->defaultValue()), false, SqlSyntaxMySql).evaluate());
            }
            break;
        case eFieldUint:
            typeName = "INT UNSIGNED";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                                                                                   (reinterpret_cast<const MetaFieldUint *>(field)->defaultValue()), false, SqlSyntaxMySql).evaluate());
            }
            break;
        case eFieldInt64:
            typeName = "BIGINT UNSIGNED";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                                                                                   (reinterpret_cast<const MetaFieldInt64 *>(field)->defaultValue()), false, SqlSyntaxMySql).evaluate());
            }
            break;
        case eFieldUint64:
            typeName = "BIGINT UNSIGNED";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                                                                                   (reinterpret_cast<const MetaFieldUint64 *>(field)->defaultValue()), false, SqlSyntaxMySql).evaluate());
            }
            break;
        case eFieldFloat:
            typeName = "FLOAT";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                                                                                   (reinterpret_cast<const MetaFieldFloat *>(field)->defaultValue()), false, SqlSyntaxMySql).evaluate());
            }
            break;
        case eFieldDouble:
            typeName = "DOUBLE";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                                                                                   (reinterpret_cast<const MetaFieldDouble *>(field)->defaultValue()), false, SqlSyntaxMySql).evaluate());
            }
            break;
        case eFieldString:
            typeName = "TEXT";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                                                                                   (reinterpret_cast<const MetaFieldString *>(field)->defaultValue()), false, SqlSyntaxMySql).evaluate());
            }
            break;
        case eFieldDateTime:
            typeName = "DATETIME";
            if (field->mandatoriness() == eDefaultable)
            {
                constraints.push_back("DEFAULT " + detail::SqlExpressionTreeWalker(std::make_shared<db::detail::ExpressionNodeImplLiteral>
                                                                                   (reinterpret_cast<const MetaFieldDateTime *>(field)->defaultValue()), false, SqlSyntaxMySql).evaluate());
            }
            break;
        default:
            throw std::runtime_error(std::string("Cannot handle field ") + field->name() + " as an sql column");
        }

        auto primaryKey = findConstraint(SqlConstraintTypePrimaryKey, field);
        if (primaryKey)
        {
            if (field->isIntegral())
                typeName += " PRIMARY KEY AUTO_INCREMENT";
            else
                throw std::runtime_error("Primary keys are only allowed on integral types");
        }

        auto foreignKey = std::dynamic_pointer_cast<SqlConstraintForeignKey>(
                    findConstraint(SqlConstraintTypeForeignKey, field));
        if (foreignKey)
        {
            constraints.push_back(String("REFERENCES ") + foreignKey->referenceMetaObject()->name() +
                                  "(" + foreignKey->referenceMetaField()->name() + ")");
        }

        auto check = findConstraint(SqlConstraintTypeCheck, field);
        if (check)
        {
            constraints.push_back("CHECK (" +
                                  std::dynamic_pointer_cast<SqlConstraintCheck>(check)->checkExpression()
                                  + ")");
        }

        String column = name + " " + typeName + (constraints.size() ? " " : "") + join(constraints, " ");
        columns.push_back(column);
    }
    queryStr += join(columns, ", ") + ")";

    try
    {
        // main statement CREATE TABLE
        SqlStatementCustom statement(queryStr);
        statement.exec(transaction);
    }
    catch (const std::exception&) {
        // skip creation of indices if schema is already exist
        return;
    }

    for (auto constraint : constraints)
    {
        if (constraint->type() == SqlConstraintTypeIndex)
        {
            if (std::dynamic_pointer_cast<SqlConstraintIndex>(constraint)->unique())
                SqlStatementCustom(String("ALTER TABLE ") + metaObject->name() + " ADD UNIQUE (" + constraint->metaField()->name() + ")").exec(transaction);
            else
                SqlStatementCustom(String("ALTER TABLE ") + metaObject->name() + " ADD INDEX (" + constraint->metaField()->name() + ")").exec(transaction);
        }
    }
}


} // namespace sql
} // namespace db
} // namespace metacpp
