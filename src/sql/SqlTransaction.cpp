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
#include "SqlTransaction.h"
#include "SqlConnectorBase.h"
#include "SqlTransactionImpl.h"

namespace metacpp
{
namespace sql
{

SqlTransaction::SqlTransaction(SqlTransactionAutoCloseMode autoClose, connectors::SqlConnectorBase *connector)
    : m_connector(connector), m_impl(nullptr), m_autoCloseMode(autoClose),
      m_transactionStarted(false)
{
    m_impl = connector->createTransaction();
    if (!m_impl)
        throw std::runtime_error("Failed to create transaction");
    if (autoClose != SqlTransactionAutoCloseManual)
        begin();
}

SqlTransaction::~SqlTransaction()
{
    if (m_transactionStarted)
    {
        switch (m_autoCloseMode)
        {
        case SqlTransactionAutoCommit:
            commit();
            break;
        case SqlTransactionAutoRollback:
            rollback();
            break;
        default:
            throw std::runtime_error("Destroing transaction with non-closed transaction");
        }
    }
    if (m_impl)
        m_connector->closeTransaction(m_impl);
}

connectors::SqlConnectorBase *SqlTransaction::connector() const
{
    return m_connector;
}

connectors::SqlTransactionImpl *SqlTransaction::impl() const
{
    return m_impl;
}

bool SqlTransaction::started() const
{
    return m_transactionStarted;
}

void SqlTransaction::begin()
{
    if (started())
        throw std::runtime_error("Transaction already started");
    if (m_impl->begin())
    {
        m_transactionStarted = true;
    }
    else
        throw std::runtime_error("Begin transaction failed");
}

void SqlTransaction::commit()
{
    if (!started())
        throw std::runtime_error("Transaction already finished");
    if (m_impl->commit())
    {
        m_transactionStarted = false;
    }
    else
        throw std::runtime_error("Commit failed");
}

void SqlTransaction::rollback()
{
    if (!started())
        throw std::runtime_error("Transaction already finished");
    if (m_impl->rollback())
    {
        m_transactionStarted = false;
    }
    else
        throw std::runtime_error("Rollback failed");
}

} // namespace sql
} // namespace metacpp
