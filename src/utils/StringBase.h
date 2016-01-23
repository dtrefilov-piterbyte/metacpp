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
#ifndef STRING_H
#define STRING_H
#include <string>
#include <stdio.h>
#include <wchar.h>
#include <sstream>
#include "Array.h"

#ifdef _MSC_VER
#define U16(str) reinterpret_cast<const char16_t *>(L##str)
#else
#define U16(str) u##str
#endif

namespace metacpp
{

template<typename T>
class StringBase;

/** \brief String in native system encoding (ANSI on Windows platforms and UTF-8 on linux)
 * \relates metacpp::StringBase
 */
typedef StringBase<char> String;

/** \brief String in UTF16-LE encoding (for use with Win32 API, JNI and others)
 * \relates metacpp::StringBase
 */
typedef StringBase<char16_t> WString;

/** \brief Performs string conversion from character buffer in system encoding
 * \relates metacpp::StringBase
 */
template<typename T>
T string_cast(const char *aString, size_t length = (size_t)-1);

/** \brief Performs string conversion from character buffer in UTF16-LE encoding
 * \relates metacpp::StringBase
 */
template<typename T>
T string_cast(const char16_t *wString, size_t length = (size_t)-1);

/** \brief Performs string conversion from String
 * \relates metacpp::StringBase
 */
template<typename T>
T string_cast(const String& strA);

/** \brief Performs string conversion from WString
 * \relates metacpp::StringBase
 */
template<typename T>
T string_cast(const WString& strW);

/** \brief Performs string conversion from std::string in system encoding
 * \relates metacpp::StringBase
 */
template<typename T>
T string_cast(const std::string& strA);

/** \brief Performs string conversion from std::basic_string in UTF16-LE encoding
 * \relates metacpp::StringBase
 */
template<typename T>
T string_cast(const std::basic_string<char16_t>& strW);

namespace detail
{

    /** \brief Unified helper for manipulating C-strings */
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
} // namespace detail

template<typename T> class InputStringStreamBase;
template<typename T> class OutputStringStreamBase;
template<typename T> class StringStreamBase;

template<typename T>
class StringBufBase : public std::basic_streambuf<T>
{
public:
    typedef typename std::basic_streambuf<T>::char_type			char_type;
    typedef typename std::basic_streambuf<T>::traits_type 		traits_type;
    typedef typename traits_type::int_type                      int_type;
    typedef typename traits_type::pos_type                      pos_type;
    typedef typename traits_type::off_type                      off_type;

    explicit StringBufBase(const StringBase<T>& s, std::ios_base::openmode which =
            std::ios_base::in | std::ios_base::out)
        : m_str(s), m_sink(nullptr)
    {
        update_ptrs(0, 0, which & std::ios_base::ate ? m_str.size() : 0);
    }

    explicit StringBufBase(std::basic_ostream<T> *sink, std::ios_base::openmode which =
            std::ios_base::in | std::ios_base::out, std::size_t initialBufSize = 32)
        : m_sink(sink)
    {
        (void)which;
        m_str.reserve(initialBufSize);
        update_ptrs(0, 0, 0);
    }

    explicit StringBufBase(std::ios_base::openmode which =
            std::ios_base::in | std::ios_base::out, std::size_t initialBufSize = 32)
        : m_sink(nullptr)
    {
        (void)which;
        m_str.reserve(initialBufSize);
        update_ptrs(0, 0, 0);
    }

    ~StringBufBase()
    {
    }

    const StringBase<T>& str()
    {
        sync();
        return m_str;
    }

private:

    StringBufBase(const StringBufBase&)=delete;
    StringBufBase& operator=(const StringBufBase&)=delete;

    // input overrides
    int_type underflow() override
    {
        sync();
        char_type *ptr = this->gptr();
        return ptr == this->egptr() ?
                    traits_type::eof() :
                    traits_type::to_int_type(*ptr);
    }

    std::streamsize showmanyc() override
    {
        return std::distance(this->gptr(), this->egptr()) - 1;
    }

    // output overrides
    int_type overflow(int_type ch) override
    {
        if ((!m_sink || *m_sink) && traits_type::eof() != ch)
        {
            append(ch);
            return ch;
        }
        return traits_type::eof();
    }

    int sync() override
    {
        char_type *beg = const_cast<char_type *>(const_cast<const StringBase<T>&>(m_str).begin());

        std::streamsize nBase = std::distance(beg, this->pbase());
        std::streamsize nCurrent = std::distance(beg, this->pptr());
        std::streamsize nInput = std::distance(beg, this->gptr());
        std::streamsize nInputEnd = std::distance(beg, this->egptr());
        if (m_sink && *m_sink)
            m_sink->write(this->pbase(), nCurrent - nBase);
        this->setp(beg + nCurrent, beg + m_str.capacity());
        m_str.resize(std::max(nCurrent, nInputEnd));
        char_type *end = const_cast<char_type *>(const_cast<const StringBase<T>&>(m_str).end());
        this->setg(beg, beg + nInput, end);
        return 0;
    }

    void append(int_type ch)
    {
        // save current buffer pointers
        std::streamsize nInputCurrent = std::distance(this->eback(), this->gptr());
        std::streamsize nOutputBase = std::distance(const_cast<char_type *>(m_str.begin()), this->pbase());
        std::streamsize nOutputCurrent = std::distance(const_cast<char_type *>(m_str.begin()), this->pptr());

        if (m_str.size() <= (size_t)nOutputCurrent)
            m_str.resize(nOutputCurrent + 1);
        m_str[nOutputCurrent++] = ch;
        update_ptrs(nInputCurrent, nOutputBase, nOutputCurrent);
    }

	void update_ptrs(size_t nInput, size_t nBase, size_t nOutputCurrent)
    {
        // avoid detach
        char_type *beg = const_cast<char_type *>(const_cast<const StringBase<T>&>(m_str).begin());
        char_type *end = const_cast<char_type *>(const_cast<const StringBase<T>&>(m_str).end());

        // set input pointers
        this->setg(beg, beg + nInput, end);
        // set output pointers
        this->setp(beg + nBase, beg + m_str.capacity());
        this->pbump((int)(nOutputCurrent - nBase));
    }

private:
    StringBase<T> m_str;
    std::basic_ostream<T> *m_sink;
};

typedef StringBufBase<char> StringBuf;
typedef StringBufBase<char16_t> WStringBuf;

template<typename T>
class InputStringStreamBase : public std::basic_istream<T>
{
public:
    explicit InputStringStreamBase(const StringBase<T>& s = StringBase<T>(),
                                   std::ios_base::openmode which = std::ios_base::in)
        : std::basic_istream<T>(&m_stringbuf), m_stringbuf(s, which)
    {
    }
private:
    StringBufBase<T> m_stringbuf;
};

typedef InputStringStreamBase<char> InputStringStream;
typedef InputStringStreamBase<char16_t> WInputStringStream;

template<typename T>
class OutputStringStreamBase : public std::basic_ostream<T>
{
public:
    explicit OutputStringStreamBase(std::ios_base::openmode which = std::ios_base::out)
        : std::basic_ostream<T>(&m_stringbuf), m_stringbuf(which)
    {
    }

    const StringBase<T>& str()
    {
        return m_stringbuf.str();
    }
private:
    StringBufBase<T> m_stringbuf;
};

typedef OutputStringStreamBase<char> OutputStringStream;
typedef OutputStringStreamBase<char16_t> WOutputStringStream;

template<typename T>
class StringStreamBase : public std::basic_iostream<T>
{
public:
    explicit StringStreamBase(const StringBase<T>& s,
                                   std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
        : std::basic_iostream<T>(&m_stringbuf), m_stringbuf(s, which)
    {
    }

    explicit StringStreamBase(std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
        : std::basic_iostream<T>(&m_stringbuf), m_stringbuf(which)
    {

    }

    const StringBase<T>& str()
    {
        return m_stringbuf.str();
    }
private:
    StringBufBase<T> m_stringbuf;
};

typedef StringStreamBase<char> StringStream;
typedef StringStreamBase<char16_t> WStringStream;

/** \brief Base template class for representing strings (in either system encoding or UTF-16 encoding)
 * Implementation uses copy-on-write optimization methods.
 */
template<typename T>
class StringBase : protected SharedDataPointer<detail::StringData<T> >
{
    typedef detail::StringData<T> Data;
    typedef  SharedDataPointer<detail::StringData<T> > Base;
public:
    /** \brief Random access iterator for accessing an individual character in the string (or supplementary and surrogate character) */
	typedef T *iterator;
    /** \brief Const random access iterator for accessing individual characters in the string (or supplementary and surrogate character) */
	typedef const T *const_iterator;
    /** \brief Represents reference to an individual character in the string (or supplementary and surrogate character) */
	typedef T& reference;
    /** \brief Represents const reference to an individual character in the string (or supplementary and surrogate character) */
	typedef const T& const_reference;
    /** \brief Invalid/end-of-string character index */
    static const size_t npos;

    /** \brief Constructs new null string */
    StringBase() { }
    /** \brief Constructs new instance of StringBase using null-terminated C-string */
    StringBase(const T *str) : Base(new Data(str)) { }
    /** \brief Constructs new instance of StringBase from the given buffer */
    StringBase(const T *str, size_t length) : Base(new Data(str, length)) { }
    /** \brief Constructs new instance of StringBase from the given range of characters */
    StringBase(const_iterator begin, const_iterator end) : StringBase(begin, std::distance(begin, end)) { }
    /** \brief Constructs new instance of StringBase from another instance */
    StringBase(const StringBase& other) : Base(other) { }
    /** \brief Constructs new instance of StringBase from standard library string */
    StringBase(const std::basic_string<T>& stdstr) : Base(new Data(stdstr.c_str(), stdstr.size())) { }
    ~StringBase() { }

    /** \brief Gets a pointer to the raw C-string buffer used to store this string */
    const T *data() const { return this->m_d ? this->m_d->_data() : ms_empty.data(); }
    /** \brief Gets a pointer to the raw C-string buffer used to store this string */
    const T *c_str() const { return this->m_d ? this->m_d->_data() : ms_empty.data(); }

    /** \brief Checks whether this string is undefined */
	bool isNull() const { return !this->m_d || !this->m_d->_data(); }
    /** \brief Checks whether this string is either undefined or empty */
	bool isNullOrEmpty() const { return isNull() || !*this->m_d->_data(); }

    /** \brief Sets new string value to this instance from given C-string */
    StringBase& operator=(const T *rhs)
    {
        this->clear();
        this->m_d = new Data(rhs);
		return *this;
	}

    /** \brief Checks two strings for equality */
    bool equals(const StringBase& rhs, bool caseSensetive = true) const
	{
		bool hasA = !isNullOrEmpty(), hasB = !rhs.isNullOrEmpty();
		if (hasA && hasB) return 0 == (caseSensetive ? this->m_d->_cmp(rhs.data()) : this->m_d->_icmp(rhs.data()));
		return hasA == hasB;
	}

    /** \brief Checks two strings for equality */
	bool equals(const T *rhs, bool caseSensetive = true)
	{
		bool hasA = !isNullOrEmpty(), hasB = rhs && *rhs;
		if (hasA && hasB) return 0 == (caseSensetive ? this->m_d->_cmp(rhs) : this->m_d->_icmp(rhs));
		return hasA == hasB;
	}

    /** \brief Gets length of this string in characters (excluding terminating null character) */
	size_t length() const { return this->m_d ? this->m_d->_length() : 0; }

    /** \brief Gets a const reference to the character at given position */
    const_reference operator[](size_t i) const { assert(i < length()); return this->m_d->_data()[i];  }
    /** \brief Gets a reference to the character at given position */
    reference operator[](size_t i) { assert(i < length()); this->detachOrInitialize(); return this->m_d->_data()[i];  }

    /** \brief Gets length of this string in characters (excluding terminating null character) */
	size_t size() const { return length(); }
    /** \brief Gets current length of the buffer used to store value of this string */
	size_t capacity() const { return this->m_d ? this->m_d->_capacity() - 1 : 0; }
    /** \brief Ensures buffer is capable to store string value of the given length */
    void reserve(size_t length) { this->detachOrInitialize(); this->m_d->_reserve(length + 1); }
    /** \brief Squeezes buffer to it's minimum possible length capable to store current string value */
    void squeeze() { this->detachOrInitialize(); this->m_d->_squeeze(); }
    /** \brief Sets string length to the given value terminating it with null character */
    void resize(size_t size) { this->detachOrInitialize(); this->m_d->_resize(size + 1); }

    /** \brief Gets a reference to the first character in the string. String should not be null or empty. */
    reference front() { assert(size()); return *begin(); }
    /** \brief Gets a const reference to the first character in the string. String should not be null or empty. */
	const_reference front() const { assert(size()); return *begin(); }
    /** \brief Gets a reference to the last character in the string. String should not be null or empty. */
	reference back() { assert(size()); return *(end() - 1); }
    /** \brief Gets a const reference to the last character in the string. String should not be null or empty. */
    const_reference back() const { assert(size()); return *(end() - 1); }

    /** \brief Gets an iterator pointing to the first character in the string. */
    iterator begin() { this->detachOrInitialize(); return isNull() ? ms_empty.begin() : this->m_d->_data(); }
    /** \brief Gets an iterator pointing to the null terminating character in the string. */
    iterator end() { this->detachOrInitialize(); return isNull() ? ms_empty.end() : (this->m_d->_data() + this->m_d->_length()); }
    /** \brief Gets a const iterator pointing to the first character in the string. */
    const_iterator begin() const { return isNull() ? ms_empty.begin() : this->m_d->_data(); }
    /** \brief Gets a const iterator pointing to the null terminating character in the string. */
    const_iterator end() const { return isNull() ? ms_empty.end() : (this->m_d->_data() + this->m_d->_length()); }

    /** \brief Appends given character buffer to this string */
	void append(const T *str, size_t length = npos)
	{
		if (str && *str)
		{
            this->detachOrInitialize();
			this->m_d->_append(str, length);
		}
	}

    /** \brief Appends one character to this string */
	void append(const T& ch) { append(&ch, 1); }

    /** \brief Appends another string to this string */
    void append(const StringBase& other) { append(other.data(), other.length()); }

    /** \brief Appends given C-style string to this string */
    StringBase& operator +=(const T *str) { append(str); return *this; }
    /** \brief Appends one character to this string */
    StringBase& operator +=(const T& ch) { append(ch); return *this; }
    /** \brief Appends another string to this string */
    StringBase& operator +=(const StringBase& str) { append(str); return *this; }

    /** \brief Gets a position of the next occurence of given character buffer in this string starting from the specified position.
     *
     * \arg str Pointer to the buffer
     * \arg pos Starting position to search from
     * \arg length Length of the buffer to search for. Calculated automatically if npos provided.
     *
     * \returns npos if nothing found
     */
    size_t nextIndexOf(const T *str, size_t pos, size_t length = npos) const
	{
        if (length == npos) length = detail::StringHelper<T>::strlen(str);
        if (pos >= size()) return npos;
        auto it = std::search(begin() + pos, end(), str, str + length);
		return it == end() ? npos : std::distance(begin(), it);
	}

    /** \brief Gets a position of the next occurence of given substring in this string starting from the specified position.
     *
     * \arg other Substring to search for
     * \arg pos Starting position to search from
     *
     * \returns npos if nothing found
     */
    size_t nextIndexOf(const StringBase& other, size_t pos) const { return nextIndexOf(other.data(), pos, other.length()); }

    /** \brief Gets a position of the first occurence of given character buffer in this string.
     *
     * \arg str Pointer to the buffer
     * \arg length Length of the buffer to search for. Calculated automatically if npos provided.
     *
     * \returns npos if nothing found
     */
    size_t firstIndexOf(const T *str, size_t length = npos) const { return nextIndexOf(str, 0, length); }

    /** \brief Gets a position of the first occurence of given character buffer in this string.
     *
     * \arg other Substring to search for
     *
     * \returns npos if nothing found
     */
    size_t firstIndexOf(const StringBase& other) const { return firstIndexOf(other.data(), other.length()); }

    /** \brief Gets a position of the last occurence of given character buffer in this string.
     *
     * \arg str Pointer to the buffer
     * \arg length Length of the buffer to search for. Calculated automatically if npos provided.
     *
     * \returns npos if nothing found
     */
	size_t lastIndexOf(const T *str, size_t length = npos) const
	{
        if (length == npos) length = detail::StringHelper<T>::strlen(str);
		auto it = std::find_end(begin(), end(), str, str + length);
		return it == end() ? npos : std::distance(begin(), it);
	}

    /** \brief Gets a position of the last occurence of given character buffer in this string.
     *
     * \arg other Substring to search for
     *
     * \returns npos if nothing found
     */
    size_t lastIndexOf(const StringBase& other) const { return lastIndexOf(other.data(), other.length()); }

    /** \brief Checks whether this string contains any occurences of given character buffer */
    bool contains(const T *str, size_t length = npos) const { return firstIndexOf(str, length) != npos; }
    /** \brief Checks whether this string contains any occurences of given substring */
    bool contains(const StringBase& other) const { return firstIndexOf(other) != npos; }

    /** \brief Checks whether the beginning of this string matches given character buffer */
	bool startsWith(const T *str, size_t length = npos) const
	{
        if (length == npos) length = detail::StringHelper<T>::strlen(str);
		if (isNullOrEmpty()) return false;
        return 0 == detail::StringHelper<T>::strncmp(begin(), str, length);
	}

    /** \brief Checks whether the beginning of this string matches given null-terminated string literal */
	template<size_t N>
	bool startsWith(const T (&str)[N]) const { return startsWith(str, N - 1); }

    /** \brief Checks whether the beginning of this string matches given string */
    bool startsWith(const StringBase& other) const { return startsWith(other.data(), other.size()); }

    /** \brief Checks whether end of this string matches given character buffer */
	bool endsWith(const T *str, size_t length = npos) const
	{
        if (length == npos) length = detail::StringHelper<T>::strlen(str);
		if (isNullOrEmpty()) return false;
        return 0 == detail::StringHelper<T>::strncmp(end() - length, str, length);
	}

    /** \brief Checks whether the end of this string matches given null-terminated string literal */
	template<size_t N>
	bool endsWith(const T (&str)[N]) const { return endsWith(str, N - 1); }

    /** \brief Checks whether the end of this string matches given string */
    bool endsWith(const StringBase& other) const { return endsWith(other.data(), other.size()); }

    /** \brief Returns an empty string */
    static const StringBase& getEmpty() { return ms_empty; }
    /** \brief Returns an null string */
    static const StringBase& getNull() { return ms_null; }

    /** \brief Transforms given value to the string */
	template<typename T1>
    static StringBase fromValue(const T1& value)
    {
        OutputStringStreamBase<T> ss;
        ss << value;
        return ss.str();
    }

    static StringBase fromValue(const StringBase& value)
    {
        return value;
    }

    /** \brief Trys to transform this string to value of the given type */
    template<typename T1>
    T1 toValue() const
    {
        T1 res;
        InputStringStreamBase<T> ss(*this);
        ss.exceptions(std::ios::failbit | std::ios::badbit);
        ss >> res;
        return res;
    }

    /** \brief Returns array of substring in this instance delimited by given seperator */
    Array<StringBase<T> > split(T separator, bool keepEmptyElements = false) const
	{
        Array<StringBase<T> > result;
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

    /** \brief Retrieves a substring from this string */
    StringBase substr(size_t start = 0, size_t length = npos) const
    {
        if (start >= size()) return StringBase();
        if (npos == length) length = size() - start;
        if (length > size() - start) length = size() - start;
        return StringBase(begin() + start, length);
    }

    /** \brief Replaces all occurences of one substring with another */
    StringBase& replace(const StringBase& from, const StringBase& to)
    {
        size_t pos = firstIndexOf(from);
        if (pos != npos) {
            StringBase newStr;
            size_t begPos = 0;
            do {
                newStr.append(begin() + begPos, pos - begPos);
                newStr.append(to);
                begPos = pos + from.size();
            } while (npos != (pos = nextIndexOf(from, begPos)));
            newStr.append(begin() + begPos, size() - begPos);
            *this = newStr;
        }
        return *this;
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

/** \brief Combines strings in this array into one string delimiting them with given string */
template<typename T>
StringBase<T> join(const Array<StringBase<T> >& arr, const T *delim = "", size_t delimSize = (size_t)-1)
{
    StringBase<T> res;
    size_t reserveSize = 0;
    if ((size_t)-1 == delimSize) delimSize = detail::StringHelper<T>::strlen(delim);
    for (size_t i = 0; i < arr.size(); ++i)
        reserveSize += arr[i].size() + delimSize;
    if (arr.size()) reserveSize -= delimSize;
    res.reserve(reserveSize);
    for (size_t i = 0; i < arr.size(); ++i)
    {
        res += arr[i];
        if (i != arr.size() - 1) res += delim;
    }
    return res;
}

template<typename T>
StringBase<T> join(const Array<StringBase<T> >& arr, const StringBase<T>& delim)
{
    return join(arr, delim.c_str(), delim.size());
}

typedef Array<String> StringArray;
typedef Array<WString> WStringArray;

template<typename T1, typename T2>
class StringBuilder;

namespace detail
{
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
} // namespace detail

/** \brief Helper class for string concatenation
 * \relates metacpp::StringBase
 */
template<typename T1, typename T2>
class StringBuilder
{
public:
    typedef typename detail::StringBuilderHelper<T1>::CharT CharT;

    StringBuilder(const detail::StringBuilderHelper<T1>& a, const detail::StringBuilderHelper<T2>& b)
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
    detail::StringBuilderHelper<T1> m_a;
    detail::StringBuilderHelper<T2> m_b;
};

/** \brief Concatenates two strings
 * \relates metacpp::StringBase
 */
template<typename T>
StringBuilder<StringBase<T>, StringBase<T> > operator +(const StringBase<T>& a, const StringBase<T>& b)
{
    return StringBuilder<StringBase<T>, StringBase<T> >(a, b);
}

/** \relates metacpp::StringBase */
template<typename T>
StringBuilder<StringBase<T>, StringBase<T> > operator +(const StringBase<T>& a, const std::basic_string<T>& b)
{
    return StringBuilder<StringBase<T>, std::basic_string<T> >(a, b);
}

/** \relates metacpp::StringBase */
template<typename T>
StringBuilder<StringBase<T>, StringBase<T> > operator +(const std::basic_string<T>& a, const StringBase<T>& b)
{
    return StringBuilder<std::basic_string<T>, StringBase<T> >(a, b);
}

/** \relates metacpp::StringBase */
template<typename T>
StringBuilder<StringBase<T>, T *> operator +(const StringBase<T>& a, const T *b)
{
    return StringBuilder<StringBase<T>, T *>(a, b);
}

/** \relates metacpp::StringBase */
template<typename T>
StringBuilder<T *, StringBase<T>> operator +(const T *a, const StringBase<T>& b)
{
    return StringBuilder<T *, StringBase<T> >(a, b);
}

/** \relates metacpp::StringBase */
template<typename T, typename T1, typename T2>
StringBuilder<StringBase<T>, StringBuilder<T1, T2> > operator +(const StringBase<T>& a, const StringBuilder<T1, T2>& b)
{
    return StringBuilder<StringBase<T>, StringBuilder<T1, T2> >(a, b);
}

/** \relates metacpp::StringBase */
template<typename T, typename T1, typename T2>
StringBuilder<StringBuilder<T1, T2>, StringBase<T>> operator +(const StringBuilder<T1, T2>& a, const StringBase<T>& b)
{
    return StringBuilder<StringBuilder<T1, T2>, StringBase<T> >(a, b);
}

/** \relates metacpp::StringBase */
template<typename T, typename T1, typename T2>
StringBuilder<StringBase<T>, StringBuilder<T1, T2> > operator +(const std::basic_string<T>& a, const StringBuilder<T1, T2>& b)
{
    return StringBuilder<std::basic_string<T>, StringBuilder<T1, T2> >(a, b);
}

/** \relates metacpp::StringBase */
template<typename T, typename T1, typename T2>
StringBuilder<StringBuilder<T1, T2>, StringBase<T>> operator +(const StringBuilder<T1, T2>& a, const std::basic_string<T>& b)
{
    return StringBuilder<StringBuilder<T1, T2>, std::basic_string<T> >(a, b);
}

/** \relates metacpp::StringBase */
template<typename T, typename T1, typename T2>
StringBuilder<T *, StringBuilder<T1, T2> > operator +(const T *a, const StringBuilder<T1, T2>& b)
{
    return StringBuilder<T *, StringBuilder<T1, T2> >(a, b);
}

/** \relates metacpp::StringBase */
template<typename T, typename T1, typename T2>
StringBuilder<StringBuilder<T1, T2>, T *> operator +(const StringBuilder<T1, T2>& a, const T *b)
{
    return StringBuilder<StringBuilder<T1, T2>, T *>(a, b);
}

/** \relates metacpp::StringBase */
template<typename T>
bool operator<(const StringBase<T>& a, const StringBase<T>& b)
{
    return detail::StringHelper<T>::strcmp(a.data(), b.data()) < 0;
}

/** \relates metacpp::StringBase */
std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const String& str);
/** \relates metacpp::StringBase */
std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const WString& wstr);
/** \relates metacpp::StringBase */
std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, const WString& str);
/** \relates metacpp::StringBase */
std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, const String& wstr);

/** \relates metacpp::StringBase */
std::basic_istream<char>& operator>>(std::basic_istream<char>& is, String& str);
std::basic_istream<char>& operator>>(std::basic_istream<char>& is, WString& wstr);
/** \relates metacpp::StringBase */
std::basic_istream<char16_t>& operator>>(std::basic_istream<char16_t>& is, WString& wstr);
/** \relates metacpp::StringBase */
std::basic_istream<char16_t>& operator>>(std::basic_istream<char16_t>& is, String& str);

/** \relates metacpp::Array */
template<typename T>
std::basic_ostream<char>& operator<<(std::basic_ostream<char>& stream, const Array<T>& a)
{
    Array<String> serialized = a.map([](const T& v) { return String::fromValue(v); });
    return stream << "[ " << join(serialized, ", ") << " ]";
}

/** \relates metacpp::Array */
template<typename T>
std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& stream, const Array<T>& a)
{
    Array<WString> serialized = a.map([](const T& v) { return WString::fromValue(v); });
    return stream << U16("[ ") << join(serialized, U16(", ")) + U16(" ]");
}

} // namespace metacpp
#endif // STRING_H
