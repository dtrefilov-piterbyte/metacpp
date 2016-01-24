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
#ifndef ARRAY_H
#define ARRAY_H
#include <algorithm>
#include <string.h>
#include "SharedDataBase.h"
#include "SharedDataPointer.h"
#include <type_traits>
#include <cassert>
#include <functional>

namespace metacpp
{

namespace detail
{
    template<typename T, typename Enable = void>
    struct TypeTraits;

    /** \brief Array traits specialization for POD types */
    template<typename T>
    struct TypeTraits<T, typename std::enable_if<std::is_pod<T>::value>::type>
    {
        static T *Allocate(size_t size, const T *initialData = nullptr)
        {
            T *data = (T *)malloc(size * sizeof(T));
            if (initialData)
                memcpy(data, initialData, size * sizeof(T));
            return data;
        }

        static void Deallocate(T *data)
        {
            free(data);
        }

        static T *Reallocate(T *data, size_t newSize, size_t oldSize)
        {
            (void)oldSize;
            return (T *)realloc(data, newSize * sizeof(T));
        }
    };

    /** \brief Array traits specialization for non-movable types */
    template<typename T>
    struct TypeTraits<T, typename std::enable_if<!std::is_pod<T>::value && !std::is_constructible<T, T&&>::value>::type>
    {
        static T *Allocate(size_t size, const T *initialData = nullptr)
        {
            T *data = new T[size];
            if (initialData) std::copy_n(initialData, size, data);
            return data;
        }

        static void Deallocate(T *data)
        {
            delete[] data;
        }

        static T *Reallocate(T *data, size_t newSize, size_t oldSize)
        {
            T *newData = newSize ? new T[newSize] : nullptr;
            if (data)
            {
                std::copy_n(data, std::min(newSize, oldSize), newData);
                delete[] data;
            }
            return newData;
        }
    };

    /** \brief Array traits specialization for movable types */
    template<typename T>
    struct TypeTraits<T, typename std::enable_if<!std::is_pod<T>::value && std::is_constructible<T, T&&>::value>::type>
    {
        static T *Allocate(size_t size, const T *initialData = nullptr)
        {
            T *data = new T[size];
            if (initialData)
                std::move(initialData, initialData + size, data);
            return data;
        }

        static void Deallocate(T *data)
        {
            delete[] data;
        }

        static T *Reallocate(T *data, size_t newSize, size_t oldSize)
        {
            T *newData = new T[newSize];
            if (data)
            {
                std::move(data, data + std::min(newSize, oldSize), newData);
                delete[] data;
            }
            return newData;
        }
    };

    typedef void *(*AllocateCB_t)(size_t size, const void *initialData);
    typedef void (*DeallocateCB_t)(void *data);
    typedef void *(*ReallocateCB_t)(void *data, size_t newSize, size_t oldSize);

    typedef struct
    {
        AllocateCB_t allocCb;
        DeallocateCB_t deallocCb;
        ReallocateCB_t reallocCb;
    } ArrayDataTraits;

    template<typename T>
    class ArrayData : public SharedDataBase
    {
    public:
        ArrayData() :
            m_data(nullptr), m_dwSize(0), m_dwAllocatedSize(0)
        {
            ArrayDataTraits traits;
            traits.allocCb = (AllocateCB_t)&TypeTraits<T>::Allocate;
            traits.deallocCb = (DeallocateCB_t)&TypeTraits<T>::Deallocate;
            traits.reallocCb = (ReallocateCB_t)&TypeTraits<T>::Reallocate;
            _initTraits(traits);
        }

        ArrayData(const ArrayDataTraits& traits) :
            m_data(nullptr), m_dwSize(0), m_dwAllocatedSize(0)
        {
            _initTraits(traits);
        }

        ArrayData(const T *data, size_t size)
        {
            ArrayDataTraits traits;
            traits.allocCb = (AllocateCB_t)&TypeTraits<T>::Allocate;
            traits.deallocCb = (DeallocateCB_t)&TypeTraits<T>::Deallocate;
            traits.reallocCb = (ReallocateCB_t)&TypeTraits<T>::Reallocate;
            _initTraits(traits);

            if (size)
            {
                m_data = (T *)m_traits.allocCb(size, data);
                if (!m_data)
                    throw std::bad_alloc();

                m_dwSize = m_dwAllocatedSize = size;
            }
            else
            {
                m_data = nullptr;
                m_dwSize = m_dwAllocatedSize = 0;
            }

        }

        ArrayData(const ArrayData&)/* =delete */;

        ~ArrayData()
        {
            if (m_data) m_traits.deallocCb(m_data);
        }

        void _initTraits(const ArrayDataTraits& traits)
        {
            m_traits = traits;
        }

        void _reserve(size_t size)
        {
            if (m_dwSize < size && m_dwAllocatedSize < size)
            {
                m_data = (T *)m_traits.reallocCb(m_data, size, m_dwSize);
                if (!m_data)
                    throw std::bad_alloc();
                m_dwAllocatedSize = size;
            }
        }

        void _resize(size_t size)
        {
            if (m_dwAllocatedSize < size)
            {
                size_t newSize = m_dwAllocatedSize + m_dwAllocatedSize * 3 / 2;
                if (newSize < size) newSize = size;

                _reserve(newSize);
            }
            m_dwSize = size;
        }

        void _squeeze()
        {
            if (m_dwAllocatedSize != m_dwSize)
            {
                m_data = (T *)m_traits.reallocCb(m_data, m_dwSize, m_dwAllocatedSize);
                if (m_dwSize && !m_data)
                    throw std::bad_alloc();
                m_dwAllocatedSize = m_dwSize;
            }
        }

        size_t _size() const
        {
            return m_dwSize;
        }

        size_t _capacity() const
        {
            return m_dwAllocatedSize;
        }

        void _free()
        {
            if (m_data) m_traits.deallocCb(m_data);
            m_dwAllocatedSize = m_dwSize = 0;
        }

        T *_data() { return m_data; }
        const T *_data() const { return m_data; }

        void _push_back(const T& v)
        {
            _resize(m_dwSize + 1);
            *(m_data + m_dwSize - 1) = v;
        }

        template<typename... TArgs>
        void _emplace_back(TArgs... args)
        {
            _resize(m_dwSize + 1);
            new (m_data + m_dwSize - 1) T(args...);
        }

        void _push_back(T&& v)
        {
            _resize(m_dwSize + 1);
            *(m_data + m_dwSize - 1) = std::move(v);
        }

        void _push_front(const T& v)
        {
            _resize(m_dwSize + 1);
            std::copy(m_data, m_data + m_dwSize - 1, m_data + 1);
            *m_data = v;
        }

        void _push_front(T&& v)
        {
            _resize(m_dwSize + 1);
            std::copy(m_data, m_data + m_dwSize - 1, m_data + 1);
            *m_data = std::move(v);
        }

        void _pop_back()
        {
            assert(m_dwSize);
            _resize(m_dwSize - 1);
        }

        void _pop_front()
        {
            assert(m_dwSize);
            std::copy(m_data + 1, m_data + m_dwSize, m_data);
            _resize(m_dwSize -1);
        }

        void _insert(const T& v, size_t i)
        {
            assert(i <= m_dwSize);
            _resize(m_dwSize + 1);
            std::copy(m_data + i, m_data + m_dwSize - 1, m_data + i + 1);
            *(m_data + i) = v;
        }

        template<typename... TArgs>
        void _emplace(size_t i, TArgs... args)
        {
            assert(i <= m_dwSize);
            _resize(m_dwSize + 1);
            std::copy(m_data + i, m_data + m_dwSize - 1, m_data + i + 1);
            new (m_data + i)T(args...);
        }

        void _erase(size_t from, size_t to)
        {
            assert(from <= m_dwSize && to <= m_dwSize && from < to);
            std::copy(m_data + to, m_data + m_dwSize, m_data + from);
            _resize(m_dwSize - (to - from));
        }

        SharedDataBase *clone() const override
        {
            ArrayData *copy = new ArrayData();
            copy->m_data = (T *)m_traits.allocCb(m_dwSize, m_data);
            if (!copy->m_data)
                throw std::bad_alloc();
            copy->m_dwAllocatedSize = copy->m_dwSize = m_dwSize;
            copy->m_traits = m_traits;
            return copy;
        }
    protected:
        T *m_data;
        size_t m_dwSize, m_dwAllocatedSize;
        ArrayDataTraits m_traits;
    };
} // namespace detail

/** \brief A template class that provides dynamic collection of simple datatypes.
 * Utilizes copy-on-write techinque.
 */
template<typename T>
class Array : protected SharedDataPointer<detail::ArrayData<T> >
{
    typedef SharedDataPointer<detail::ArrayData<T> > Base;
public:
    /** \brief Random access STL iterator for this array */
	typedef T *iterator;
    /** \brief Const random access STL iterator for this array */
	typedef const T *const_iterator;
    /** \brief Reference to the element of array */
	typedef T& reference;
    /** \brief Const reference to the element of array */
	typedef const T& const_reference;

    /** \brief Constructs a new empty array */
    Array()
        : Base(new detail::ArrayData<T>())
    {
	}

    /** \brief Constructs a new array from the existing. Both arrays shares the same data buffer
     * until someone will not try to change one of them  */
    Array(const Array& o) : Base(o)
	{
    }

    /** \brief Constructs a new array and initializes it's data from raw buffer */
    Array(const T *data, size_t size)
        : Base(new detail::ArrayData<T>(data, size))
	{
	}

    /** \brief Constructs a new array and initializes it with braced initializer list */
    Array(const std::initializer_list<T>& init)
    {
        reserve(init.size());
        for (auto& item : init)
            push_back(item);
    }

    ~Array()
	{
	}

    /** \brief Gets the pointer to the raw buffer */
    T *data() { this->detachOrInitialize(); return this->m_d->_data(); }
    /** \brief Gets the pointer to the readonly raw buffer */
	const T *data() const { return this->m_d ? this->m_d->_data() : nullptr; }
    /** \brief Gets number of elements in the array */
	size_t size() const { return this->m_d ? this->m_d->_size() : 0; }
    /** \brief Checks whether array is empty (i.e. either uninitialized or having no elements) */
	bool empty() const { return 0 == size(); }
    /** \brief Gets maximum number of arguments that may be fitted into this array without need of expanding buffers */
	size_t capacity() const { return this->m_d ? this->m_d->_capacity() : 0; }
    /** \brief Ensures that array may fit given number of arguments */
    void reserve(size_t size) { this->detachOrInitialize(); this->m_d->_reserve(size); }
    /** \brief Resizes array to the given number of arguments.
     *
     * If current size of the array exceeds given number, the array is truncted to the specified value,
     * if array size is smaller than given size, the array is expanded with default constructed elements
     */
    void resize(size_t size) { this->detachOrInitialize(); this->m_d->_resize(size); }
    /** \brief Sqeezes allocated buffers to the minimum size to fit array data */
    void squeeze() { this->detachOrInitialize(); this->m_d->_squeeze(); }
    /** \brief Removes one element at the specified position */
    void erase(size_t i) { this->detachOrInitialize(); this->m_d->_erase(i, i + 1); }
    /** \brief Removes element in the inclusive range between specified positions */
    void erase(size_t from, size_t to) { this->detachOrInitialize(); this->m_d->_erase(from, to); }
    /** \brief Removes one element pointed by specified iterator */
    void erase(const_iterator it) { erase(it - begin()); }

    /** \brief Gets reference to the element at specified index */
    reference operator[](size_t i) { this->detachOrInitialize(); assert(i < size()); return this->m_d->_data()[i]; }
    /** \brief Gets const reference to the element at specified index */
	const_reference operator[](size_t i) const { assert(i < size()); return this->m_d->_data()[i]; }

    /** \brief Gets reference to the first element in the array. Array should not be empty. */
	reference front() { assert(size()); return *begin(); }
    /** \brief Gets const reference to the first element in the array. Array should not be empty. */
	const_reference front() const { assert(size()); return *begin(); }
    /** \brief Gets reference to the last element in the array. Array should not be empty. */
	reference back() { assert(size()); return *(end() - 1); }
    /** \brief Gets const reference to the last element in the array. Array should not be empty. */
	const_reference back() const { assert(size()); return *(end() - 1); }

    /** \brief Gets an STL iterator pointing to the begin of this array */
    iterator begin() { this->detachOrInitialize(); return this->m_d->_data(); }
    /** \brief Gets an STL iterator pointing to the end of this array */
    iterator end() { this->detachOrInitialize(); return this->m_d->_data() + this->m_d->_size(); }
    /** \brief Gets an const STL iterator pointing to the begin of this array */
	const_iterator begin() const { return this->m_d->_data(); }
    /** \brief Gets an const STL iterator pointing to the end of this array */
	const_iterator end() const { return this->m_d->_data() + this->m_d->_size(); }

    /** \brief Puts given element into the end of this array. Operation has complexity O(1). */
    void push_back(const T& v) { this->detachOrInitialize(); this->m_d->_push_back(v); }
    /** \brief Puts given element into the begin of this array. Operation has complexity O(N), where N is a current array size. */
    void push_front(const T& v) { this->detachOrInitialize(); this->m_d->_push_front(v); }
    /** \brief Removes element from the end of this array. Operation has complexity O(1) */
    void pop_back() { this->detachOrInitialize(); this->m_d->_pop_back(); }
    /** \brief Removes element from the begin of this array. Operation has complexity O(N), where N is a current array size. */
    void pop_front() { this->detachOrInitialize(); this->m_d->_pop_front(); }

    /** \brief Constructs element in-place at the end of this array */
    template<typename... TArgs>
    void emplace_back(TArgs... args) { this->detachOrInitialize(); this->m_d->_emplace_back(args...); }

    /** \brief Puts set of elements from an array into the end of this array */
    void append(const T *many, size_t n) {
        this->detachOrInitialize();
        reserve(size() + n);
        for (size_t i = 0; i < n; ++i) this->m_d->_push_back(many[i]);
    }

    /** \brief Concatenates this array with other */
    void append(const Array& other) { this->append(other.data(), other.size()); }

    /** \brief Empties this array */
    void clear() { resize(0); squeeze(); }

    /** \brief Applies functor to each element in this array and returns a new array of results */
    template<typename TRes>
    Array<TRes> map(const std::function<TRes (const T&)>& functor) const
    {
        Array<TRes> res;
        res.reserve(size());
        for (size_t i = 0; i < size(); ++i)
            res.emplace_back(functor((*this)[i]));
        return res;
    }
};

/** \brief Array of bytes
 * \relates metacpp::Array
 */
typedef Array<uint8_t> ByteArray;

} // namespace metacpp
#endif // ARRAY_H
