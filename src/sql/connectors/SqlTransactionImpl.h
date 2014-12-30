#ifndef SQLTRANSACTIONIMPL_H
#define SQLTRANSACTIONIMPL_H
#include "SqlStorable.h"
#include "SqlStatementImpl.h"

namespace metacpp
{
namespace sql
{
namespace connectors
{

class SqlTransactionImpl
{
protected:
    SqlTransactionImpl();
public:
    SqlTransactionImpl(const SqlTransactionImpl&)=delete;
    SqlTransactionImpl& operator=(const SqlTransactionImpl&)=delete;

    virtual ~SqlTransactionImpl();

    /** \brief Create a statement */
    virtual SqlStatementImpl *createStatement(SqlStatementType type, const String& queryText) = 0;

    /** \brief Prepare statement */
    virtual bool prepare(SqlStatementImpl *statement) = 0;

    /** \brief Execute statement */
    virtual bool execStatement(SqlStatementImpl *statement) = 0;

    /** \brief Write result of select operaation into storable and move cursor to the next row */
    virtual bool fetchNext(SqlStatementImpl *statement, SqlStorable *storable) = 0;

    /** \brief Destroy statement */
    virtual bool closeStatement(SqlStatementImpl *statement) = 0;

};

} // namespace connectors
} // namespace sql
} // namespace metacpp


#endif // SQLTRANSACTIONIMPL_H
