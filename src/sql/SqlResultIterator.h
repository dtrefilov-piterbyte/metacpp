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
#ifndef SQLRESULTITERATOR_H
#define SQLRESULTITERATOR_H
#include "Object.h"

namespace metacpp
{
namespace sql
{

class SqlStorable;
class SqlResultSetData;

static const int ROW_ID_PAST_THE_END = -1;
static const int ROW_ID_INVALID = -2;

/** Forward iterator for iterating over sql result rows */
class SqlResultIterator
{
public:
    explicit SqlResultIterator(SqlResultSetData *resultSet, int rowId);
    SqlResultIterator(const SqlResultIterator& other);
    virtual ~SqlResultIterator();

    SqlResultIterator& operator=(const SqlResultIterator& rhs);
    bool operator==(const SqlResultIterator& rhs);
    bool operator!=(const SqlResultIterator& rhs);

    int operator*() const;
    int operator->() const;

    SqlResultIterator& operator++();
    inline int rowId() const { return m_rowId; }
private:
    inline void setRowId(int rowId) { m_rowId = rowId; }
private:
    friend class SqlResultSetData;
    SqlResultSetData *m_resultSet;
    int m_rowId;

};

} // namespace sql
} // namespace metacpp

#endif // SQLRESULTITERATOR_H
