#include "SharedDataBase.h"

namespace orm
{

SharedDataBase::SharedDataBase()
    : m_count(1)
{
}

SharedDataBase::~SharedDataBase()
{
}

int SharedDataBase::ref() const
{
    return ++m_count;
}

int SharedDataBase::deref() const
{
    return --m_count;
}

int SharedDataBase::count() const
{
    return m_count.load();
}

}
