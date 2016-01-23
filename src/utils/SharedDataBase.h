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
#ifndef SHAREDDATABASE_H
#define SHAREDDATABASE_H
#include <atomic>
#include <type_traits>
#include <memory>

namespace metacpp
{

/**
    \brief SharedDataBase represents an implicitly shared object
    commonly used via SharedDataPointer
*/
class SharedDataBase
{
public:
    virtual ~SharedDataBase();

    /** \brief increment internal reference counter of the object */
    int ref() const;
    /** \brief decrement reference counter */
    int deref() const;
    /** \brief returns current usage count */
    int count() const;
    /** \brief create a deep copy of the object */
    virtual SharedDataBase *clone() const = 0;
protected:
    template<typename>
    friend class SharedDataPointer;
    template<typename>
    friend class SharedObjectPointer;
    SharedDataBase();
    SharedDataBase(const SharedDataBase&)=delete;
private:
    mutable std::atomic<int> m_count;
};

template<typename TObj>
class SharedObjectDataBase : public SharedDataBase
{
public:
    ~SharedObjectDataBase() = default;

    virtual TObj *get() const = 0;
    virtual void reset(TObj *) = 0;
    virtual TObj *extract() = 0;
};

template<typename TObj, typename Deleter = std::default_delete<TObj> >
class SharedObjectData : public SharedObjectDataBase<TObj>
{
public:
    explicit SharedObjectData(TObj *pObj = nullptr, const Deleter deleter = Deleter())
        : m_obj(pObj), m_deleter(deleter)
    {
    }

    SharedDataBase *clone() const
    {
        throw std::runtime_error("Cannot clone SharedDataBase");
        //return new SharedObjectData(m_obj, m_deleter);
    }

    ~SharedObjectData()
    {
        if (m_obj)
            m_deleter(m_obj);
    }

    TObj *get() const override { return m_obj; }

    void reset(TObj *obj) override {
        if (m_obj)
            m_deleter(m_obj);
        m_obj = obj;
    }

    TObj *extract() override
    {
        TObj *result = m_obj;
        m_obj = nullptr;
        return result;
    }

private:
    TObj *m_obj;
    Deleter m_deleter;
};

} // namespace metacpp
#endif // SHAREDDATABASE_H
