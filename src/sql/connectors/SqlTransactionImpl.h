#ifndef SQLTRANSACTIONIMPL_H
#define SQLTRANSACTIONIMPL_H
#include "SqlStorable.h"
#include "SqlStatementBase.h"

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
    virtual SqlStatementBase *createStatement(SqlStatementType type, const String& queryText) = 0;

    /** \brief Prepare statement */
    virtual bool prepare(SqlStatementBase *statement) = 0;

    /** \brief Bind object fields to the statement arguments
     * Arguments order and count in statement should match field in the object
    */
    virtual bool bindArguments(SqlStatementBase *statement, SqlStorable *storable) = 0;

    /** \brief Execute statement */
    virtual bool execStatement(SqlStatementBase *statement) = 0;

    /** \brief Write result of select operaation into storable and move cursor to the next row */
    virtual bool fetchNext(SqlStatementBase *statement, SqlStorable *storable) = 0;

    /** \brief Destroy statement */
    virtual bool closeStatement(SqlStatementBase *statement) = 0;

};

} // namespace connectors
} // namespace sql
} // namespace metacpp


#endif // SQLTRANSACTIONIMPL_H
