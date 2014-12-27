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
namespace connectors
{

enum SqlStatementType
{
    eSqlStatementTypeUnknown,
    eSqlStatementTypeSelect,
    eSqlStatementTypeInsert,
    eSqlStatementTypeUpdate,
    eSqlStatementTypeDelete,
};

/**
    \brief
*/
class SqlStatementBase
{
protected:
    SqlStatementBase(SqlStatementType type, const String& queryText);
public:
    SqlStatementBase(const SqlStatementBase&)=delete;
    virtual ~SqlStatementBase();

    /** \brief Get statement type */
    virtual SqlStatementType type() const;
    /** \brief Check if statement is prepared */
    virtual bool prepared() const;
    /** \brief Set prepared flag */
    virtual void setPrepared(bool val = true);
    /** \brief Check if we've done executing this statement (no more rows) */
    virtual bool done() const;
    /** \brief Set done flag */
    virtual void setDone(bool val = true);
    /** \brief Get sql query text */
    virtual const String& queryText() const;
protected:
    bool m_prepared;
    String m_queryText;
    SqlStatementType m_type;
    bool m_done;
};

} // namespace connectors
} // namespace sql
} // namespace metacpp

#endif // SQLSTATEMENTBASE_H

