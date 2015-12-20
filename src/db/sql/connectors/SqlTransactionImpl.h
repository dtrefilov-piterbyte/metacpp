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
#ifndef SQLTRANSACTIONIMPL_H
#define SQLTRANSACTIONIMPL_H
#include "SqlStorable.h"
#include "SqlStatementImpl.h"

namespace metacpp
{
namespace db
{
namespace sql
{
namespace connectors
{

/** \brief An abstract base class for execution of sql-statements in a context of transaction.
 *
 * This class should never be used directly
 */
class SqlTransactionImpl
{
protected:
    SqlTransactionImpl();
public:
    SqlTransactionImpl(const SqlTransactionImpl&)=delete;
    SqlTransactionImpl& operator=(const SqlTransactionImpl&)=delete;

    virtual ~SqlTransactionImpl();

    /** \brief start a transaction */
    virtual bool begin() = 0;

    /** \brief execute all commands and make all changes made within given transaction persistent */
    virtual bool commit() = 0;

    /** \brief cancel all changes made within given transaction */
    virtual bool rollback() = 0;

    /** \brief Create a statement */
    virtual SqlStatementImpl *createStatement(SqlStatementType type, const String& queryText) = 0;

    /** \brief Prepare statement */
    virtual bool prepare(SqlStatementImpl *statement, size_t numParams) = 0;

    /** \brief Bind in sql statement literal values */
    virtual bool bindValues(SqlStatementImpl *statement, const VariantArray& values) = 0;

    /** \brief Execute statement
     * \param numRowsAffected pointer to retrieve number of rows affected with update or delete statement
    */
    virtual bool execStatement(SqlStatementImpl *statement, int *numRowsAffected = nullptr) = 0;

    /** \brief Write result of select operation into storable and move cursor to the next row */
    virtual bool fetchNext(SqlStatementImpl *statement, SqlStorable *storable) = 0;

    /** \brief Returns number of rows in a result set, returns (size_t)-1 if unavailable */
    virtual size_t size(SqlStatementImpl *statement) = 0;

    /** \brief Retrieve an id of the previously inserted row */
    virtual bool getLastInsertId(SqlStatementImpl *statement, SqlStorable *storable) = 0;

    /** \brief Destroys statement */
    virtual bool closeStatement(SqlStatementImpl *statement) = 0;

};

} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp


#endif // SQLTRANSACTIONIMPL_H
