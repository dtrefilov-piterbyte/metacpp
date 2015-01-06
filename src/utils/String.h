#ifndef STRING_H
#define STRING_H
#include <string>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <sstream>
#include "Array.h"

namespace metacpp
{

template<typename T>
struct StringHelper
{
    static size_t strlen(const T *str);
    static int strcmp(const T *a, const T *b);
    static int strcasecmp(const T *a, const T *b);
    static int strncmp(const T *a, const T *b, size_t size);
    static int strncasecmp(const T *a, const T *b, size_t size);
    static T *strcpy(T *dest, const T *source) ;
    static T *strncpy(T *dest, const T *source, size_t n);
    static const T *strstr(const T *haystack, const T *needle);
};

template<typename LT, typename RT>
class StringBuilder;

template<typename T>
class StringBase;

template<typename T>
class StringArrayBase;

/** \brief String in native system encoding (ANSI on Windows platforms and UTF-8 on linux) */
typedef StringBase<char> String;
/** \brief String in UTF16-LE encoding (for use with Win32 API, JNI and others) */
typedef StringBase<char16_t> WString;

/** \brief Performs string encoding conversion */
template<typename T>
T string_cast(const char *aString, size_t length = (size_t)-1);

template<typename T>
T string_cast(const char16_t *wString, size_t length = (size_t)-1);

template<typename T>
T string_cast(const String& strA);

template<typename T>
T string_cast(const WString& strW);

template<typename T>
class StringData : public ArrayData<T>
{
public:
	typedef StringHelper<T> Helper;
	static const size_t npos;

    StringData()
		: m_dwLength(0)
	{
	}

    explicit StringData(const T *data, size_t length = npos)
        : ArrayData<T>(data, length == npos ? (data ? Helper::strlen(data) + 1 : 0) : length + 1),
		m_dwLength(this->m_dwSize ? this->m_dwSize - 1 : 0)
	{
		if (this->m_data) this->m_data[m_dwLength] = T(0);
	}

    StringData(const StringData&)/* =delete */;

    SharedDataBase *clone() const override
	{
        StringData *copy = new StringData();
		copy->m_data = new T[m_dwLength + 1];
		std::copy(this->m_data, this->m_data + m_dwLength + 1, copy->m_data);
		copy->m_dwLength = m_dwLength;
		copy->m_dwAllocatedSize = copy->m_dwSize = m_dwLength + 1;

		return copy;
	}

	void _resize(size_t size) 
	{
        ArrayData<T>::_resize(size);
		m_dwLength = this->m_dwSize ? this->m_dwSize - 1 : 0;
		this->m_data[m_dwLength] = T(0);
	}

	size_t _length() const { return m_dwLength;  }
	int _cmp(const T *o) { return this->m_data ? Helper::strcmp(this->m_data, o) : -1; }
    int _icmp(const T *o) { return this->m_data ? Helper::strcasecmp(this->m_data, o) : -1; }

	void _append(const T *data, size_t length = npos)
	{
		if (length == npos) length = Helper::strlen(data);
        ArrayData<T>::_resize(m_dwLength + length + 1);
		std::copy(data, data + length, this->m_data + m_dwLength);
		m_dwLength += length;
		this->m_data[m_dwLength] = T(0);	// null terminator
	}
private:
	size_t m_dwLength;
};

template<typename T>
class StringBase : public SharedDataPointer<StringData<T> >
{
    typedef StringData<T> Data;
    typedef  SharedDataPointer<StringData<T> > Base;
public:
	typedef T *iterator;
	typedef const T *const_iterator;
	typedef T& reference;
	typedef const T& const_reference;
    static const size_t npos;

    StringBase() { }
    StringBase(const T *str) : Base(new Data(str)) { }
    StringBase(const T *str, size_t length) : Base(new Data(str, length)) { }
    StringBase(const StringBase& other) : Base(other) { }
    StringBase(const std::basic_string<T>& stdstr) : Base(new Data(stdstr.c_str(), stdstr.size())) { }
    ~StringBase() { }

	const T *data() const { return this->m_d ? this->m_d->_data() : empty().data(); }
	const T *c_str() const { return this->m_d ? this->m_d->_data() : empty().data(); }

	bool isNull() const { return !this->m_d || !this->m_d->_data(); }
	bool isNullOrEmpty() const { return isNull() || !*this->m_d->_data(); }

    StringBase& operator=(const T *rhs)
    {
        this->clear();
        this->m_d = new Data(rhs);
		return *this;
	}

    bool equals(const StringBase& rhs, bool caseSensetive = true) const
	{
		bool hasA = !isNullOrEmpty(), hasB = !rhs.isNullOrEmpty();
		if (hasA && hasB) return 0 == (caseSensetive ? this->m_d->_cmp(rhs.data()) : this->m_d->_icmp(rhs.data()));
		return hasA == hasB;
	}

	bool equals(const T *rhs, bool caseSensetive = true)
	{
		bool hasA = !isNullOrEmpty(), hasB = rhs && *rhs;
		if (hasA && hasB) return 0 == (caseSensetive ? this->m_d->_cmp(rhs) : this->m_d->_icmp(rhs));
		return hasA == hasB;
	}

	size_t length() const { return this->m_d ? this->m_d->_length() : 0; }

	const T& operator[](size_t i) const { assert(i < length()); return this->m_d->_data()[i];  }
	T& operator[](size_t i) { assert(i < length()); this->detach(); return this->m_d->_data()[i];  }

	size_t size() const { return length(); }
	size_t capacity() const { return this->m_d ? this->m_d->_capacity() - 1 : 0; }
	void reserve(size_t size) { this->detach(); this->m_d->_reserve(size + 1); }
	void squeeze() { this->detach(); this->m_d->_squeeze(); }
	void resize(size_t size) { this->detach(); this->m_d->_resize(size + 1); }

	reference front() { assert(size()); return *begin(); }
	const_reference front() const { assert(size()); return *begin(); }
	reference back() { assert(size()); return *(end() - 1); }
	const_reference back() const { assert(size()); return *(end() - 1); }

	iterator begin() { this->detach(); return this->m_d->_data(); }
	iterator end() { this->detach(); return this->m_d->_data() + this->m_d->_length(); }
	const_iterator begin() const { return this->m_d->_data(); }
	const_iterator end() const { return this->m_d->_data() + this->m_d->_length(); }

	void append(const T *str, size_t length = npos)
	{
		if (str && *str)
		{
			this->detach();
			this->m_d->_append(str, length);
		}
	}

	void append(const T& ch) { append(&ch, 1); }

    void append(const StringBase& other) { append(other.data(), other.length()); }

    StringBase& operator +=(const T *str) { append(str); return *this; }
    StringBase& operator +=(const StringBase& str) { append(str); return *this; }
    StringBase& operator +=(const T& ch) { append(ch); return *this; }

	size_t firstIndexOf(const T *str, size_t length = npos) const
	{
		if (length == npos) length = StringHelper<T>::strlen(str);
		auto it = std::search(begin(), end(), str, str + length);
		return it == end() ? npos : std::distance(begin(), it);
	}

    size_t firstIndexOf(const StringBase& other) const { return firstIndexOf(other.data(), other.length()); }

	size_t lastIndexOf(const T *str, size_t length = npos) const
	{
		if (length == npos) length = StringHelper<T>::strlen(str);
		auto it = std::find_end(begin(), end(), str, str + length);
		return it == end() ? npos : std::distance(begin(), it);
	}

    size_t lastIndexOf(const StringBase& other) const { return lastIndexOf(other.data(), other.length()); }

	bool contains(const T *str, size_t length = npos) const { return firstIndexOf(str, length) != npos; }
    bool contains(const StringBase& other) const { return firstIndexOf(other) != npos; }

	bool startsWith(const T *str, size_t length = npos) const
	{
		if (length == npos) length = StringHelper<T>::strlen(str);
		if (isNullOrEmpty()) return false;
		return 0 == StringHelper<T>::strncmp(begin(), str, length);
	}

	template<size_t N>
	bool startsWith(const T (&str)[N]) const { return startsWith(str, N - 1); }

    bool startsWith(const StringBase& other) const { return startsWith(other.data(), other.size()); }

	bool endsWith(const T *str, size_t length = npos) const
	{
		if (length == npos) length = StringHelper<T>::strlen(str);
		if (isNullOrEmpty()) return false;
		return 0 == StringHelper<T>::strncmp(end() - length, str, length);
	}

	template<size_t N>
	bool endsWith(const T (&str)[N]) const { return endsWith(str, N - 1); }

    bool endsWith(const StringBase& other) const { return endsWith(other.data(), other.size()); }

    static const StringBase& empty() { return ms_empty; }
    static const StringBase& null() { return ms_null; }

	template<typename T1>
    static StringBase fromValue(T1 value)
	{
		std::ostringstream ss;
		ss << value;
		const std::string& s = ss.str();
        return string_cast<StringBase>(s.c_str(), s.length());
	}

    template<typename T1>
    T toValue() const
    {
        T1 res;
        std::istringstream ss(this->c_str());
        ss.exceptions(std::ios::failbit | std::ios::badbit);
        ss >> res;
        return res;
    }

    StringArrayBase<T> split(T separator, bool keepEmptyElements = false) const
	{
        StringArrayBase<T> result;
		result.reserve(20);
		auto b = begin(), e = end();
		while (true)
		{
			auto ps = std::find(b, e, separator);
            StringBase<T> elem(b, std::distance(b, ps));
			if (keepEmptyElements || !elem.isNullOrEmpty()) result.push_back(elem);
			if (ps == e) break;
			b = ps + 1;
		}
		return result;
	}

private:
    static StringBase<T> ms_null;
    static StringBase<T> ms_empty;
};

template<typename T>
StringBase<T> StringBase<T>::ms_null;

template<typename T>
inline bool operator==(const StringBase<T>& lhs, const StringBase<T>& rhs) { return lhs.equals(rhs); }

template<typename T>
inline bool operator!=(const StringBase<T>& lhs, const StringBase<T>& rhs) { return !lhs.equals(rhs); }

template<typename T>
inline bool operator==(const StringBase<T>& lhs, const T *rhs) { return lhs.equals(rhs); }

template<typename T>
inline bool operator!=(const StringBase<T>& lhs, const T *rhs) { return !lhs.equals(rhs); }

template<typename T>
inline bool operator==(const T *lhs, const StringBase<T>& rhs) { return rhs.equals(lhs); }

template<typename T>
inline bool operator!=(const T *lhs, const StringBase<T>& rhs) { return !rhs.equals(lhs); }

template<typename T>
class StringArrayBase : public Array<StringBase<T> >
{
public:
    StringArrayBase()
    {

    }

    StringArrayBase(const Array<StringBase<T> >& o) : Array<StringBase<T> >(o)
    {
    }

    StringArrayBase(const StringBase<T> *data, size_t size)
        : Array<StringBase<T> >(data, size)
    {
    }

    StringArrayBase(const std::initializer_list<StringBase<T> >& init)
        : Array<StringBase<T> >(init)
    {
    }

    StringBase<T> join(const StringBase<T> delim = StringBase<T>()) const
    {
        StringBase<T> res;
        size_t reserveSize = 0;
        for (size_t i = 0; i < this->size(); ++i)
            reserveSize += (*this)[i].size() + delim.size();
        if (this->size()) reserveSize -= delim.size();
        res.reserve(reserveSize);
        for (size_t i = 0; i < this->size(); ++i)
        {
            res += (*this)[i];
            if (i != this->size() - 1) res += delim;
        }
        return res;
    }
};

typedef StringArrayBase<char> StringArray;
typedef StringArrayBase<char16_t> WStringArray;


/** \brief Helper class for copy-on-write string concatenation */
template<typename T1, typename T2>
class StringBuilder;

template<typename T>
class StringBuilderHelper;

template<typename T>
class StringBuilderHelper<T *>
{
public:
	typedef T CharT;

    StringBuilderHelper(const T *str) : m_str(str), m_length(StringHelper<T>::strlen(m_str))
	{
	}

	size_t length() const { return m_length; }
    void appendTo(StringBase<CharT>& acc) const { acc.append(m_str, m_length); }
private:
	const T *m_str;
	size_t m_length;
};

template<typename T, size_t N>
class StringBuilderHelper<T[N]>
{
public:
	typedef T CharT;

    StringBuilderHelper(const T (&str)[N]) : m_str(str)
	{
	}

	size_t length() const { return N - 1; }
    void appendTo(StringBase<CharT>& acc) const { acc.append(m_str, N - 1); }
private:
	const T *m_str;
};

template<typename T>
class StringBuilderHelper<StringBase<T> >
{
public:
	typedef T CharT;

    StringBuilderHelper(const StringBase<T>& str)
		: m_str(str)
	{
	}

	size_t length() const { return m_str.length(); }
    void appendTo(StringBase<CharT>& acc) const { acc.append(m_str); }
private:
    const StringBase<T>& m_str;
};

template<typename T>
class StringBuilderHelper<std::basic_string<T> >
{
public:
    typedef T CharT;

    StringBuilderHelper(const std::basic_string<T>& str)
        : m_str(str)
    {
    }

    size_t length() const { return m_str.length(); }
    void appendTo(StringBase<CharT>& acc) const { acc.append(m_str.c_str(), m_str.length()); }
private:
    const std::basic_string<T>& m_str;
};

template<typename T1, typename T2>
class StringBuilderHelper<StringBuilder<T1, T2> >
{
public:
    typedef typename StringBuilder<T1, T2>::CharT CharT;

    StringBuilderHelper(const StringBuilder<T1, T2>& sb)
		: m_sb(sb)
	{
	}

	size_t length() const { return m_sb.length(); }
    void appendTo(StringBase<CharT>& acc) const { m_sb.appendTo(acc); }
private:
    const StringBuilder<T1, T2>& m_sb;
};

template<typename T1, typename T2>
class StringBuilder
{
public:
    typedef typename StringBuilderHelper<T1>::CharT CharT;

    StringBuilder(const StringBuilderHelper<T1>& a, const StringBuilderHelper<T2>& b)
		: m_a(a), m_b(b)
	{
	}

	size_t length() const { return m_a.length() + m_b.length(); }
    void appendTo(StringBase<CharT>& acc) const
	{
		m_a.appendTo(acc);
		m_b.appendTo(acc);
	}

    operator StringBase<CharT>() const
	{
        StringBase<CharT> result;
		size_t length = this->length();
		result.reserve(length);
		appendTo(result);
		return result;
	}
private:
    StringBuilderHelper<T1> m_a;
    StringBuilderHelper<T2> m_b;
};

template<typename T>
StringBuilder<StringBase<T>, StringBase<T> > operator +(const StringBase<T>& a, const StringBase<T>& b)
{
    return StringBuilder<StringBase<T>, StringBase<T> >(a, b);
}

template<typename T>
StringBuilder<StringBase<T>, StringBase<T> > operator +(const StringBase<T>& a, const std::basic_string<T>& b)
{
    return StringBuilder<StringBase<T>, std::basic_string<T> >(a, b);
}

template<typename T>
StringBuilder<StringBase<T>, StringBase<T> > operator +(const std::basic_string<T>& a, const StringBase<T>& b)
{
    return StringBuilder<std::basic_string<T>, StringBase<T> >(a, b);
}

template<typename T>
StringBuilder<StringBase<T>, T *> operator +(const StringBase<T>& a, const T *b)
{
    return StringBuilder<StringBase<T>, T *>(a, b);
}

template<typename T>
StringBuilder<T *, StringBase<T>> operator +(const T *a, const StringBase<T>& b)
{
    return StringBuilder<T *, StringBase<T> >(a, b);
}

template<typename T, typename T1, typename T2>
StringBuilder<StringBase<T>, StringBuilder<T1, T2> > operator +(const StringBase<T>& a, const StringBuilder<T1, T2>& b)
{
    return StringBuilder<StringBase<T>, StringBuilder<T1, T2> >(a, b);
}

template<typename T, typename T1, typename T2>
StringBuilder<StringBuilder<T1, T2>, StringBase<T>> operator +(const StringBuilder<T1, T2>& a, const StringBase<T>& b)
{
    return StringBuilder<StringBuilder<T1, T2>, StringBase<T> >(a, b);
}

template<typename T, typename T1, typename T2>
StringBuilder<StringBase<T>, StringBuilder<T1, T2> > operator +(const std::basic_string<T>& a, const StringBuilder<T1, T2>& b)
{
    return StringBuilder<std::basic_string<T>, StringBuilder<T1, T2> >(a, b);
}

template<typename T, typename T1, typename T2>
StringBuilder<StringBuilder<T1, T2>, StringBase<T>> operator +(const StringBuilder<T1, T2>& a, const std::basic_string<T>& b)
{
    return StringBuilder<StringBuilder<T1, T2>, std::basic_string<T> >(a, b);
}

template<typename T, typename T1, typename T2>
StringBuilder<T *, StringBuilder<T1, T2> > operator +(const T *a, const StringBuilder<T1, T2>& b)
{
    return StringBuilder<T *, StringBuilder<T1, T2> >(a, b);
}

template<typename T, typename T1, typename T2>
StringBuilder<StringBuilder<T1, T2>, T *> operator +(const StringBuilder<T1, T2>& a, const T *b)
{
    return StringBuilder<StringBuilder<T1, T2>, T *>(a, b);
}

template<typename T>
bool operator<(const StringBase<T>& a, const StringBase<T>& b)
{
    return StringHelper<T>::strcmp(a.data(), b.data()) < 0;
}

std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const String& str);
std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const WString& wstr);
std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, const WString& str);
std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, const String& wstr);

std::basic_istream<char>& operator>>(std::basic_istream<char>& os, const String& str);
std::basic_istream<char>& operator>>(std::basic_istream<char>& os, const WString& wstr);
std::basic_istream<char16_t>& operator>>(std::basic_istream<char16_t>& os, const WString& str);
std::basic_istream<char16_t>& operator>>(std::basic_istream<char16_t>& os, const String& wstr);

} // namespace metacpp
#endif // STRING_H
