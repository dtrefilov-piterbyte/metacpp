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

    // return a record
    const Object *operator*() const;
    const Object *operator->() const;

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
