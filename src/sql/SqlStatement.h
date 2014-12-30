#ifndef SQLSTATEMENT_H
#define SQLSTATEMENT_H
#include "SqlStorable.h"
#include "SqlWhereClause.h"
#include "SqlTransaction.h"
#include "SqlResultSet.h"

namespace metacpp
{
namespace sql
{

enum SqlStatementType
{
    eSqlStatementTypeUnknown,
    eSqlStatementTypeSelect,
    eSqlStatementTypeInsert,
    eSqlStatementTypeUpdate,
    eSqlStatementTypeDelete,
};

/** \brief Base class for all common-types statement builders */
class SqlStatementBase
{
    explicit SqlStatementBase(SqlStorable *storable);
    SqlStatementBase(const SqlStatementBase&)=delete;
    SqlStatementBase& operator=(const SqlStatementBase&)=delete;
public:
    virtual ~SqlStatementBase();
    virtual SqlStatementType type() const = 0;
protected:
    SqlStorable *m_storable;
};

class SqlStatementSelect : public SqlStatementBase
{
public:
    explicit SqlStatementSelect(SqlStorable *storable);
    ~SqlStatementSelect();

    SqlStatementType type() const override;

    /** construct inner join statement */
    SqlStatementSelect& innerJoin(...); // pass typed table list somehow
    /** construct left outer join statement */
    SqlStatementSelect& outerJoin(...);
    SqlStatementSelect& limit(size_t lim);
    SqlStatementSelect& offset(size_t off);
    SqlStatementSelect& where(const WhereClauseBuilder& whereClause);
    SqlResultSet exec(SqlTransaction& transaction);
};


class SqlStatementInsert : public SqlStatementBase
{
public:
    explicit SqlStatementInsert(SqlStorable *storable);
    ~SqlStatementInsert();

    SqlStatementType type() const override;

    void exec(SqlTransaction& transaction);
};

class SqlStatementUpdate : public SqlStatementBase
{
public:
    explicit SqlStatementUpdate(SqlStorable *storable);
    ~SqlStatementUpdate();

    SqlStatementType type() const override;

    /** reference other tables */
    SqlStatementUpdate& from(...);
    SqlStatementUpdate& where(const WhereClauseBuilder& whereClause);
    void exec(SqlTransaction& transaction);
};

class SqlStatementDelete : public SqlStatementBase
{
public:
    explicit SqlStatementDelete(SqlStorable *storable);
    ~SqlStatementDelete();

    /** reference other tables */
    SqlStatementUpdate& from(...);
    SqlStatementUpdate& where(const WhereClauseBuilder& whereClause);
    void exec(SqlTransaction& transaction);
};


} // namespace sql
} // namespace metacpp

#endif // SQLSTATEMENT_H
