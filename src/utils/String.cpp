#include "String.h"
#include <climits>

#ifdef _WIN32
#include <windows.h>

// brokes numeric limits
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#else // _WIN32
#include <iconv.h>
#include <cstdio>
#endif // _WIN32

namespace metacpp
{

    template<>
    size_t StringHelper<char>::strlen(const char *str) {
        return ::strlen(str);
    }

    template<>
    int StringHelper<char>::strcmp(const char *a, const char *b) {
        return ::strcmp(a, b);
    }

    template<>
    int StringHelper<char>::strcasecmp(const char *a, const char *b) {
#ifdef _MSC_VER
        return ::stricmp(a, b);
#else
        return ::strcasecmp(a, b);
#endif
    }

    template<>
    int StringHelper<char>::strncmp(const char *a, const char *b, size_t size) {
        return ::strncmp(a, b, size);
    }

    template<>
    int StringHelper<char>::strncasecmp(const char *a, const char *b, size_t size) {
#ifdef _MSC_VER
        return ::strnicmp(a, b, size);
#else
        return ::strncasecmp(a, b, size);
#endif
    }

    template<>
    char *StringHelper<char>::strcpy(char *dest, const char *source) {
        return ::strcpy(dest, source);
    }

    template<>
    char *StringHelper<char>::strncpy(char *dest, const char *source, size_t n) {
        return ::strncpy(dest, source, n);
    }


#ifdef _WIN32
    template<>
    size_t StringHelper<char16_t>::strlen(const char16_t *str) {
        return ::wcslen(str);
    }

    template<>
    int StringHelper<char16_t>::strcmp(const char16_t *a, const char16_t *b) {
        return ::wcscmp(a, b);
    }

    template<>
    int StringHelper<char16_t>::strcasecmp(const char16_t *a, const char16_t *b) {
#ifdef _MSC_VER
        return ::wcsicmp(a, b);
#else
        return ::wcscasecmp(a, b);
#endif
    }

    template<>
    int StringHelper<char16_t>::strncmp(const char16_t *a, const char16_t *b, size_t size) {
        return ::wcsncmp(a, b, size);
    }

    template<>
    int StringHelper<char16_t>::strnicmp(const char16_t *a, const char16_t *b, size_t size) {
#ifdef _MSC_VER
        return ::wcsnicmp(a, b, size);
#else
        return ::wcsncasecmp(a, b, size);
#endif
    }

    template<>
    char16_t *StringHelper<char16_t>::strcpy(char16_t *dest, const char16_t *source) {
        return ::wcscpy(dest, source);
    }

    template<>
    char16_t *StringHelper<char16_t>::strncpy(char16_t *dest, const char16_t *source, size_t n) {
        return ::wcsncpy(dest, source, n);
    }
};
#else
    template<>
    size_t StringHelper<char16_t>::strlen(const char16_t *str)
    {
        size_t result = 0;
        while (*str++) ++result;
        return result;
    }

    template<>
    int StringHelper<char16_t>::strcmp(const char16_t *a, const char16_t *b)
    {
        while (true)
        {
            if (*a != *b) return (int)*a - (int)*b;
            if (!*a) return 0;
            ++a; ++b;
        }
    }

    template<>
    int StringHelper<char16_t>::strcasecmp(const char16_t *a, const char16_t *b)
    {
        std::locale loc;
        while (true)
        {
            int ca = std::tolower(*a, loc), cb = std::tolower(*b, loc);
            if (ca != cb) return ca - cb;
            if (!ca) return 0;
            ++a; ++b;
        }
    }

    template<>
    int StringHelper<char16_t>::strncmp(const char16_t *a, const char16_t *b, size_t size)
    {
        while (true)
        {
            if (!size--) break;
            if (*a != *b) return (int)*a - (int)*b;
            if (!*a) return 0;
            ++a; ++b;
        }
        return 0;
    }

    template<>
    int StringHelper<char16_t>::strncasecmp(const char16_t *a, const char16_t *b, size_t size)
    {
        std::locale loc;
        while (true)
        {
            if (!size--) break;
            int ca = std::tolower(*a, loc), cb = std::tolower(*b, loc);
            if (ca != cb) return ca - cb;
            if (!ca) return 0;
            ++a; ++b;
        }
    }

    template<>
    char16_t *StringHelper<char16_t>::strcpy(char16_t *dest, const char16_t *source)
    {
        char16_t *result = dest;
        while (*source) *dest++ = *source++;
        return result;
    }

    template<>
    char16_t *StringHelper<char16_t>::strncpy(char16_t *dest, const char16_t *source, size_t n)
    {
        char16_t *result = dest;
        while (*source && n--) *dest++ = *source++;
        return result;
    }

#endif

    template<> const size_t StringData<char>::npos = std::numeric_limits<size_t>::max();
    template<> const size_t StringBase<char>::npos = std::numeric_limits<size_t>::max();

    template<> const size_t StringData<char16_t>::npos = std::numeric_limits<size_t>::max();
    template<> const size_t StringBase<char16_t>::npos = std::numeric_limits<size_t>::max();

    template<>
    StringBase<char> StringBase<char>::ms_empty("");

    template<>
    StringBase<char16_t> StringBase<char16_t>::ms_empty(u"");

	template<>
    String string_cast<String>(const char *aString, size_t length) {
        return String(aString, length);
	}

	template<>
    WString string_cast<WString>(const char16_t *wString, size_t length) {
		
        return WString(wString, length);
	}

	template<>
    WString string_cast<WString>(const char *aString, size_t length) {
#ifdef _WIN32
        size_t resultLength = MultiByteToWideChar(CP_ACP, 0, aString, length, NULL, 0) - 1;
        WString result(nullptr, resultLength);
        MultiByteToWideChar(CP_ACP, 0, aString, length, const_cast<char16_t *>(result.data()), resultLength + 1);
        return result;
#else
        WString result;
        iconv_t cd = iconv_open("UTF-16LE", "UTF-8");
        if ((iconv_t)-1 == cd)
        {
            perror("iconv_open");
            return WString();
        }

        size_t inputSize = (size_t)-1 != length ? length : StringHelper<char>::strlen(aString);

        size_t bufLength = inputSize;
        size_t converted = -1;
        do {
            result.reserve(bufLength);

            size_t inputBytesLeft = inputSize;
            size_t outputBytesLeft = bufLength * sizeof(char16_t);
            char *inbuf = const_cast<char *>(aString);
            char *outbuf = (char *)result.data();
            converted = iconv(cd, &inbuf, &inputBytesLeft, &outbuf, &outputBytesLeft);
            if ((size_t)-1 == converted && E2BIG == errno)
            {
                bufLength += (size_t)(0.3 * bufLength + 1); // increase buffer
                continue;
            }

            // truncate string
            result.resize(bufLength - outputBytesLeft / sizeof(char16_t));
            break;
        } while (true);

        if (-1 == iconv_close(cd)) {
            perror("iconv_close");
            return WString();
        }
        return result;
#endif
	}

	template<>
    String string_cast<String>(const char16_t *wString, size_t length) {
#ifdef _WIN32
        size_t resultLength = WideCharToMultiByte(CP_ACP, 0, wString, length, NULL, 0, NULL, NULL) - 1;
        String result(nullptr, resultLength)
        WideCharToMultiByte(CP_ACP, 0, wString, -1,  const_cast<char *>(result.data()), resultLength + 1, NULL, NULL);
        return result;
#else

        String result;
        iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
        if ((iconv_t)-1 == cd)
        {
            perror("iconv_open");
            return String();
        }

        size_t inputSize = (size_t)-1 != length ? length : StringHelper<char16_t>::strlen(wString);

        size_t bufLength = inputSize;
        size_t converted = -1;
        do {
            result.reserve(bufLength);

            size_t inputBytesLeft = inputSize * sizeof(char16_t);
            size_t outputBytesLeft = bufLength;
            char *inbuf = (char *)wString;
            char *outbuf = const_cast<char *>(result.data());
            converted = iconv(cd, &inbuf, &inputBytesLeft, &outbuf, &outputBytesLeft);
            if ((size_t)-1 == converted && E2BIG == errno)
            {
                bufLength += (size_t)(0.3 * bufLength + 1); // increase buffer
                continue;
            }

            // truncate string
            result.resize(bufLength - outputBytesLeft);
            break;
        } while (true);

        if (-1 == iconv_close(cd)) {
            perror("iconv_close");
            return String();
        }
        return result;
#endif
	}
	
	template<> 
    String string_cast<String>(const String& strA) {
		return strA;
    }

    template<>
    WString string_cast<WString>(const String& strA)
    {
        return string_cast<WString>(strA.data(), strA.length());
    }

    template<>
    String string_cast<String>(const WString& strW)
    {
        return string_cast<String>(strW.data(), strW.length());
    }

	template<> 
    WString string_cast<WString>(const WString& strW)
	{
		return strW;
	}

    std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const String& str)
    {
        return os << str.data();
    }

    std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const WString& wstr)
    {
        return os << string_cast<String>(wstr);
    }

    std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, const WString& str)
    {
        return os << str.data();
    }

    std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, const String& wstr)
    {
        return os << string_cast<WString>(wstr);
    }
}
