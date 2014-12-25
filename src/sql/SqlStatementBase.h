#ifndef SQLSTATEMENTBASE_H
#define SQLSTATEMENTBASE_H
#include <iostream>
#include <memory>
#include "String.h"
#include "Utils.h"

namespace metacpp
{
namespace sql
{

enum SqlStatementType
{
    eSqlStatementTypeUnknown,
    eSqlStatementTypeSelect,
    eSqlStatementTypeUpdate,
    eSqlStatementTypeDelete,
};

class SqlStatementBase
{
protected:
    SqlStatementBase(const String& queryText);
public:
    SqlStatementBase(const SqlStatementBase&)=delete;
    virtual ~SqlStatementBase();

    virtual SqlStatementType type() const;
    virtual bool prepared() const;
    virtual void setPrepared(bool val = true);
    virtual const String& queryText() const;
protected:
    bool m_prepared;
    String m_queryText;
    SqlStatementType m_type;
};

} // namespace sql
} // namespace metacpp

#endif // SQLSTATEMENTBASE_H

