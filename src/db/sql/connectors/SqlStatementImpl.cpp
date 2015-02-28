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
#include "SqlStatementImpl.h"

namespace metacpp
{
namespace db
{
namespace sql
{
namespace connectors
{


SqlStatementImpl::SqlStatementImpl(SqlStatementType type, const String& queryText)
    : m_prepared(false), m_queryText(queryText), m_type(type), m_done(false)
{

}

SqlStatementImpl::~SqlStatementImpl()
{

}

SqlStatementType SqlStatementImpl::type() const
{
    return m_type;
}

bool SqlStatementImpl::prepared() const
{
    return m_prepared;
}

void SqlStatementImpl::setPrepared(bool val)
{
    m_prepared = val;
}

bool SqlStatementImpl::done() const
{
    return m_done;
}

void SqlStatementImpl::setDone(bool val)
{
    m_done = val;
}

const String& SqlStatementImpl::queryText() const
{
    return m_queryText;
}

} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp
