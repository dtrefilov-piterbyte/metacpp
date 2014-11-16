#pragma once
#include <atomic>
#include <type_traits>

namespace orm
{

template<typename T> class SharedDataPointer;
template<typename T> class SharedPointer;

/**
    \brief \c pkSharedDataBase represents an implicitly shared object
    commonly used via \c SharedDataPointer
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
    SharedDataBase(const SharedDataBase&) /*=delete*/;
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

} // namespace pkapi
