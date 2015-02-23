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
#ifndef SHAREDDATAPOINTER_H
#define SHAREDDATAPOINTER_H
#include "SharedDataBase.h"
#include <algorithm>

namespace metacpp
{
    /** \brief Template class holding a shared reference to SharedDataBase */
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
            clear();
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

        SharedDataPointer& operator=(const T *d)
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

        T *data() const { return m_d; }

        SharedDataPointer& swap(const SharedDataPointer& o)
        {
            std::swap(m_d, o.m_d);
        }

        void clear()
        {
            if (m_d && !m_d->deref())
                delete m_d;
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

    /** \brief Template class holding a shared reference to T */
    template<typename T>
    class SharedPointer final : private SharedDataPointer<SharedData<T> >
    {
    public:
        SharedPointer()
        {
        }

        explicit SharedPointer(const T& from)
            : SharedDataPointer<SharedData<T> >(new SharedData<T>(from))
        {
        }

        template<typename... TArgs>
        explicit SharedPointer(TArgs... args)
            : SharedDataPointer<SharedData<T> >(new SharedData<T>(args...))
        {
        }

        T *operator*() const { return &this->m_d->m_data; }
        T *operator->() const { return &this->m_d->m_data; }
    };

    template<typename T, typename... TArgs>
    SharedPointer<T> MakeShared(TArgs... args)
    {
        return SharedPointer<T>(args...);
    }

} // namespace metacpp
#endif // SHAREDDATAPOINTER_H
