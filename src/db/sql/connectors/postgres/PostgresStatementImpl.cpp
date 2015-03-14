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
#include "PostgresStatementImpl.h"

namespace metacpp {
namespace db
{
namespace sql {
namespace connectors {
namespace postgres {

PostgresStatementImpl::PostgresStatementImpl(SqlStatementType type, const String &queryText)
    : SqlStatementImpl(type, queryText), m_result(nullptr), m_currentRow(-1)
{

}

PostgresStatementImpl::~PostgresStatementImpl()
{
    if (prepared() && m_result)
        PQclear(m_result);
}

void PostgresStatementImpl::setResult(PGresult *result, const String &idString)
{
    m_result = result;
    m_idString = idString;
}

PGresult *PostgresStatementImpl::getResult() const
{
    return m_result;
}

const String &PostgresStatementImpl::getIdString() const
{
    return m_idString;
}

int PostgresStatementImpl::currentRow() const
{
    return m_currentRow;
}

void PostgresStatementImpl::setCurrentRow(int row)
{
    m_currentRow = row;
}

void PostgresStatementImpl::bindValues(const VariantArray &values)
{
    m_boundValues = values;
}

const VariantArray &PostgresStatementImpl::boundValues() const
{
    return m_boundValues;
}

} // namespace postgres
} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp

