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
#ifndef SQLTRANSACTION_H
#define SQLTRANSACTION_H
#include "SqlConnectorBase.h"

namespace metacpp
{
namespace sql
{

namespace connectors
{
    class SqlConnectorBase;
    class SqlTransactionImpl;
}

enum SqlTransactionAutoCloseMode
{
    SqlTransactionAutoRollback,
    SqlTransactionAutoCommit,
    SqlTransactionAutoCloseManual
};

class SqlTransaction
{
public:
    SqlTransaction(SqlTransactionAutoCloseMode autoClose = SqlTransactionAutoRollback,
                   connectors::SqlConnectorBase *connector = connectors::SqlConnectorBase::getDefaultConnector());

    virtual ~SqlTransaction();

    SqlTransaction(const SqlTransaction&)=delete;
    SqlTransaction operator=(const SqlTransaction&)=delete;

    connectors::SqlConnectorBase *connector() const;
    connectors::SqlTransactionImpl *impl() const;
    bool started() const;
    void begin();
    void commit();
    void rollback();
private:
    connectors::SqlConnectorBase *m_connector;
    connectors::SqlTransactionImpl *m_impl;
    SqlTransactionAutoCloseMode m_autoCloseMode;
    bool m_transactionStarted;
};


} // namespace sql
} // namespace metacpp

#endif // SQLTRANSACTION_H
