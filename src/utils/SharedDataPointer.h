#pragma once
#include "SharedDataBase.h"
#include <algorithm>

namespace orm
{
	template<typename T>
    class SharedDataPointer {

	public:
		typedef T Type;

        SharedDataPointer()
			: m_d(nullptr)
		{
		}

        SharedDataPointer(const SharedDataPointer& other)
			: m_d(other.m_d)
		{
			if (m_d) m_d->ref();
		}

        virtual ~SharedDataPointer()
		{
			if (m_d && !m_d->deref()) delete m_d;
		}

        inline bool operator==(const SharedDataPointer& rhs) const { return m_d == rhs.m_d; }
        inline bool operator!=(const SharedDataPointer& rhs) const { return m_d == rhs.m_d; }
		inline bool operator==(const T *rhs) const { return m_d == rhs; }
		inline bool operator!=(const T *rhs) const { return m_d == rhs; }

		// use copy-on-write technique
		inline T& operator*() { detach(); return *m_d; }
		inline const T& operator*() const { return *m_d; }
		inline T& operator->() { detach(); return *m_d; }
		inline const T& operator->() const { return *m_d; }

        SharedDataPointer& operator=(const SharedDataPointer& o)
		{
			if (o.m_d != m_d)
			{
				if (m_d && ! m_d->deref()) delete m_d;
				m_d = o.m_d;
				if (m_d) m_d->ref();
			}
			return *this;
		}

        SharedDataPointer& operator=(const T * d)
		{
			if (d != m_d)
			{
				if (m_d && ! m_d->deref()) delete m_d;
				m_d = d;
				if (m_d) m_d->ref();
			}
			return *this;
		}

		int refCount() const { return m_d ? m_d->count() : 0; }

	protected:
        explicit SharedDataPointer(T *data) : m_d(data)
		{
		}

        SharedDataPointer& swap(const SharedDataPointer& o)
        {
            std::swap(m_d, o.m_d);
        }

        void clear()
        {
            if (m_d) m_d->deref();
            m_d = nullptr;
        }

        /**
            \brief performs a deep copy of the shared data and sets it as a private data
        */
        void detach()
        {
            if (m_d)
            {
                if (m_d->count() == 1) return;
                T *new_d = reinterpret_cast<T *>(m_d->clone());
                if (!m_d->deref()) delete m_d;
                m_d = new_d;
            }
            else
                m_d = new T();
        }

		T *m_d;
	};

	template<typename T>
    class SharedPointer : public SharedDataPointer<SharedData<T> >
	{
        typedef SharedDataPointer<SharedData<T> > Base;
	public:
        SharedPointer()
		{
		}

        explicit SharedPointer(const T& data)
            : Base(new SharedData<T>(data))
		{
		}

        SharedPointer(const SharedPointer& other)
			: Base(other)
		{
		}

		inline T& operator*() { this->detach(); return this->m_d->m_data; }
		inline const T& operator*() const { return this->m_d->m_data; }
		inline T *operator->() { this->detach(); return &this->m_d->m_data; }
		inline const T *operator->() const { return &this->m_d->m_data; }
	};
} // namespace pkapi
