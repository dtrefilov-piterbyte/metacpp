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

namespace metacpp
{

template<typename T> class SharedDataPointer;
template<typename T> class SharedPointer;

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
    template<typename T>
    friend class SharedDataPointer;
    SharedDataBase();
    SharedDataBase(const SharedDataBase&)=delete;
private:
    mutable std::atomic<int> m_count;
};

template<typename T, typename = typename std::enable_if<std::is_default_constructible<T>::value && std::is_copy_constructible<T>::value>::type>
class SharedData : public SharedDataBase
{
    template<typename T1>
    friend class SharedPointer;
public:
    SharedDataBase *clone() const override
    {
        return new SharedData<T>(m_data);
    }

    SharedData(const T& data = T())
        : m_data(data)
    {
    }

    ~SharedData()
    {
    }

private:
    T m_data;
};

} // namespace metacpp
#endif // SHAREDDATABASE_H
