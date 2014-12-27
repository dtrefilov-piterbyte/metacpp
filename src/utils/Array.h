#ifndef ARRAY_H
#define ARRAY_H
#include <algorithm>
#include "SharedDataBase.h"
#include "SharedDataPointer.h"
#include <type_traits>
#include <string.h>
#include <cassert>

namespace metacpp
{
template<typename T, typename Enable = void>
struct TypeTraits;

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
        return (T *)realloc(data, newSize * sizeof(T));
    }
};

template<typename T>
struct TypeTraits<T, typename std::enable_if<!std::is_pod<T>::value>::type>
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
		T *newData = new T[newSize];
		if (data)
		{
			std::copy_n(data, std::min(newSize, oldSize), newData);
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

	void _push_front(const T& v)
	{
		_resize(m_dwSize + 1);
		std::copy(m_data, m_data + m_dwSize - 1, m_data + 1);
		*m_data = v;
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
		copy->m_dwAllocatedSize = copy->m_dwSize = m_dwSize;
        copy->m_traits = m_traits;
		return copy;
	}
protected:
	T *m_data;
	size_t m_dwSize, m_dwAllocatedSize;
    ArrayDataTraits m_traits;
};

template<typename T>
class Array : public SharedDataPointer<ArrayData<T> >
{
    typedef SharedDataPointer<ArrayData<T> > Base;
    friend class JsonSerializerVisitor;
public:
	typedef T *iterator;
	typedef const T *const_iterator;
	typedef T& reference;
	typedef const T& const_reference;

    Array()
        : Base(new ArrayData<T>())
	{
	}

    Array(const Array& o) : Base(o)
	{
    }

    explicit Array(const T *data, size_t size)
        : Base(new ArrayData<T>(data, size))
	{
	}

    ~Array()
	{
	}

	T *data() { this->detach(); return this->m_d->_data(); }
	const T *data() const { return this->m_d ? this->m_d->_data() : nullptr; }
	size_t size() const { return this->m_d ? this->m_d->_size() : 0; }
	bool empty() const { return 0 == size(); }
	size_t capacity() const { return this->m_d ? this->m_d->_capacity() : 0; }
	void reserve(size_t size) { this->detach(); this->m_d->_reserve(size); }
	void resize(size_t size) { this->detach(); this->m_d->_resize(size); }
	void squeeze() { this->detach(); this->m_d->_squeeze(); }
	void erase(size_t i) { this->detach(); this->m_d->_erase(i, i + 1); } 
	void erase(size_t from, size_t to) { this->detach(); this->m_d->_erase(from, to); } 
    void erase(const_iterator it) { erase(it - begin()); }

	reference operator[](size_t i) { this->detach(); assert(i < size()); return this->m_d->_data()[i]; }
	const_reference operator[](size_t i) const { assert(i < size()); return this->m_d->_data()[i]; }

	reference front() { assert(size()); return *begin(); }
	const_reference front() const { assert(size()); return *begin(); }
	reference back() { assert(size()); return *(end() - 1); }
	const_reference back() const { assert(size()); return *(end() - 1); }

	iterator begin() { this->detach(); return this->m_d->_data(); }
	iterator end() { this->detach(); return this->m_d->_data() + this->m_d->_size(); }
	const_iterator begin() const { return this->m_d->_data(); }
	const_iterator end() const { return this->m_d->_data() + this->m_d->_size(); }

	void push_back(const T& v) { this->detach(); this->m_d->_push_back(v); }
	void push_front(const T& v) { this->detach(); this->m_d->_push_front(v); }
	void pop_back() { this->detach(); this->m_d->_pop_back(); }
	void pop_front() { this->detach(); this->m_d->_pop_front(); }

    void clear() { resize(0); }
};

} // namespace metacpp
#endif // ARRAY_H
