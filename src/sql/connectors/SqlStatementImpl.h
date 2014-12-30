#ifndef SQLSTATEMENTBASE_H
#define SQLSTATEMENTBASE_H
#include <iostream>
#include <memory>
#include "String.h"
#include "Utils.h"
#include "SqlStatement.h"

namespace metacpp
{
namespace sql
{
namespace connectors
{

/**
    \brief Base class for all statements
*/
class SqlStatementImpl
{
protected:
    SqlStatementImpl(SqlStatementType type, const String& queryText);
public:
    SqlStatementImpl(const SqlStatementImpl&)=delete;
    virtual ~SqlStatementImpl();

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

