#ifndef SQLSTATEMENTBASE_H
#define SQLSTATEMENTBASE_H
#include <iostream>
#include <memory>
#include "String.h"
#include "Utils.h"

namespace metacpp
{
namespace sql
{
class SqlStatementBuilderBase;

enum SqlStatementType
{
    SqlStatementTypeSelect,
    SqlStatementTypeUpdate,
    SqlStatementTypeDelete
};

class SqlStatementBase
{
public:
    SqlStatementBase();
    virtual ~SqlStatementBase();

    virtual SqlStatementType type() const = 0;
    virtual String buildQuery(SqlStatementBuilderBase *builder) const = 0;
};

typedef std::shared_ptr<SqlStatementBase> SqlStatementBasePtr;

class SelectSqlStatement;
class UpdateSqlStatement;
class DeleteSqlStatement;

class SqlStatementFactory : public FactoryBase<SqlStatementBasePtr, SqlStatementType>
{
    SqlStatementBasePtr createInstance(SqlStatementType type);
};

} // namespace sql
} // namespace metacpp

#endif // SQLSTATEMENTBASE_H

