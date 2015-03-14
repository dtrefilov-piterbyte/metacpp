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
#ifndef POSTGRESSTATEMENTIMPL_H
#define POSTGRESSTATEMENTIMPL_H
#include "SqlStatementImpl.h"
#include <libpq-fe.h>
#include <pg_config.h>

namespace metacpp {
namespace db {
namespace sql {
namespace connectors {
namespace postgres {

class PostgresStatementImpl : public SqlStatementImpl
{
public:
    PostgresStatementImpl(SqlStatementType type, const String& queryText);
    ~PostgresStatementImpl();

    void setResult(PGresult *result, const String& idString);
    PGresult *getResult() const;
    const String& getIdString() const;
    int currentRow() const;
    void setCurrentRow(int row);
    void bindValues(const VariantArray& values);
    const VariantArray& boundValues() const;
private:
    VariantArray m_boundValues;
    PGresult *m_result;
    String m_idString;
    int m_currentRow;
};

} // namespace postgres
} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp

#endif // POSTGRESSTATEMENTIMPL_H
