#include "SqlStorable.h"
#include "Nullable.h"
#include "SqlTransaction.h"

namespace metacpp
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
    int nRows = statement.exec(transaction);
    return nRows < 0 || nRows == 1;
}

bool SqlStorable::updateOne(SqlTransaction &transaction)
{
    SqlStatementUpdate statement(this);
    int nRows = statement.where(DirectWhereClauseBuilder(whereId())).exec(transaction);
    return nRows < 0 || nRows == 1;
}

bool SqlStorable::removeOne(SqlTransaction &transaction)
{
    SqlStatementDelete statement(this);
    int nRows = statement.where(DirectWhereClauseBuilder(whereId())).exec(transaction);
    return nRows < 0 || nRows == 1;
}

String SqlStorable::fieldValue(const MetaField *field) const
{
#define _FIELD_VAL_ARITH(type) \
    if (field->nullable()) \
    { \
        if (!field->access<Nullable<type> >(const_cast<SqlStorable *>(this)->record())) \
            return "NULL"; \
        else \
            return String::fromValue(*field->access<Nullable<type> >(const_cast<SqlStorable *>(this)->record())); \
    } \
    return String::fromValue(field->access<type>(const_cast<SqlStorable *>(this)->record()));

    switch(field->type())
    {
    case eFieldBool:
        _FIELD_VAL_ARITH(bool)
    case eFieldInt:
        _FIELD_VAL_ARITH(int32_t)
    case eFieldUint:
    case eFieldEnum:
        _FIELD_VAL_ARITH(uint32_t)
    case eFieldInt64:
        _FIELD_VAL_ARITH(int64_t)
    case eFieldUint64:
        _FIELD_VAL_ARITH(uint64_t)
    case eFieldFloat:
        _FIELD_VAL_ARITH(float)
    case eFieldDouble:
        _FIELD_VAL_ARITH(double)
    case eFieldString:
        if (field->nullable())
        {
            if (!field->access<Nullable<String> >(const_cast<SqlStorable *>(this)->record()))
                return "NULL";
            else
                return "\'" + *field->access<Nullable<String> >(const_cast<SqlStorable *>(this)->record()) + "\'";
        }
        return "\'" + field->access<String>(const_cast<SqlStorable *>(this)->record()) + "\'";
    case eFieldObject:
        throw std::runtime_error("Can store only plain objects");
    case eFieldArray:
        throw std::runtime_error("Can store only plain objects");
    case eFieldDateTime:
        if (field->nullable())
        {
            if (!field->access<Nullable<DateTime> >(const_cast<SqlStorable *>(this)->record()))
                return "NULL";
            else
                return "\'" + field->access<Nullable<DateTime> >(const_cast<SqlStorable *>(this)->record()).get().toISOString() + "\'";
        }
        return "\'" + field->access<DateTime>(const_cast<SqlStorable *>(this)->record()).toISOString() + "\'";
    default:
        throw std::runtime_error("Unknown field type");
    }
}

void SqlStorable::createSchema(SqlTransaction &transaction, const MetaObject *metaObject,
                               const Array<SqlConstraintBasePtr> &constraints)
{
    SqlSyntax syntax = transaction.connector()->sqlSyntax();
    if (SqlSyntaxSqlite == syntax)
        return createSchemaSqlite(transaction, metaObject, constraints);
    else if (SqlSyntaxPostgreSQL == syntax)
        return createSchemaPostgreSQL(transaction, metaObject, constraints);
    throw std::runtime_error("SqlStorable::createSchema(): syntax not implemented");
}

String SqlStorable::whereId()
{
    auto pkey = primaryKey();
    if (!pkey)
        throw std::runtime_error(std::string("Table ") + record()->metaObject()->name() +
                                 " has no primary key");
    String res = String(record()->metaObject()->name()) + "." + pkey->name() + " = " +
            fieldValue(pkey);
    return res;
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
    auto findConstraint = [constraints](SqlConstraintType type, const MetaField *field)
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
        const MetaField *field = metaObject->field(i);
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
                ValueEvaluator<bool> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldBool *>(field)->defaultValue()));
            }
            break;
        case eFieldInt:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<int32_t> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldInt *>(field)->defaultValue()));
            }
            break;
        case eFieldEnum:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<uint32_t> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldEnum *>(field)->defaultValue()));
            }
            break;
        case eFieldUint:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<uint32_t> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldUint *>(field)->defaultValue()));
            }
            break;
        case eFieldInt64:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<int64_t> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldInt64 *>(field)->defaultValue()));
            }
            break;
        case eFieldUint64:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<uint64_t> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldUint64 *>(field)->defaultValue()));
            }
            break;
        case eFieldFloat:
            typeName = "REAL";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<float> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldFloat *>(field)->defaultValue()));
            }
            break;
        case eFieldDouble:
            typeName = "REAL";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<double> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldDouble *>(field)->defaultValue()));
            }
            break;
        case eFieldString:
            typeName = "TEXT";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<String> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldString *>(field)->defaultValue()));
            }
            break;
        case eFieldDateTime:
            typeName = "DATETIME";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<DateTime> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldDateTime *>(field)->defaultValue()));
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

        String column = name + " " + typeName + (constraints.size() ? " " : "") + constraints.join(" ");
        columns.push_back(column);
    }
    queryStr += columns.join(", ") + ")";

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
    auto findConstraint = [constraints](SqlConstraintType type, const MetaField *field)
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
        const MetaField *field = metaObject->field(i);
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
                ValueEvaluator<bool> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldBool *>(field)->defaultValue()));
            }
            break;
        case eFieldInt:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<int32_t> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldInt *>(field)->defaultValue()));
            }
            break;
        case eFieldEnum:
            typeName = "INTEGER";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<uint32_t> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldEnum *>(field)->defaultValue()));
            }
            break;
        case eFieldUint:
            typeName = "BIGINT";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<uint32_t> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldUint *>(field)->defaultValue()));
            }
            break;
        case eFieldInt64:
            typeName = "BIGINT";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<int64_t> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldInt64 *>(field)->defaultValue()));
            }
            break;
        case eFieldUint64:
            typeName = "BIGINT";    // NUMERIC?
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<uint64_t> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldUint64 *>(field)->defaultValue()));
            }
            break;
        case eFieldFloat:
            typeName = "REAL";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<float> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldFloat *>(field)->defaultValue()));
            }
            break;
        case eFieldDouble:
            typeName = "DOUBLE PRECISION";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<double> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldDouble *>(field)->defaultValue()));
            }
            break;
        case eFieldString:
            typeName = "TEXT";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<String> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldString *>(field)->defaultValue()));
            }
            break;
        case eFieldDateTime:
            typeName = "TIMESTAMP";
            if (field->mandatoriness() == eDefaultable)
            {
                ValueEvaluator<DateTime> eval;
                constraints.push_back("DEFAULT " + eval(reinterpret_cast<const MetaFieldDateTime *>(field)->defaultValue()));
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

        String column = name + " " + typeName + (constraints.size() ? " " : "") + constraints.join(" ");
        columns.push_back(column);
    }
    queryStr += columns.join(", ") + ")";

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


} // namespace sql
} // namespace metacpp
