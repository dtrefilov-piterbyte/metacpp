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
#include "StringBase.h"
#include "Variant.h"
#include <climits>
#include <locale>
#include <iomanip>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>

// compatibility workaround
#define snprintf _snprintf

// breaks numeric limits
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#else

#include <iconv.h>

#endif // _WIN32

namespace metacpp
{
namespace detail
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
        return ::_stricmp(a, b);
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
        return ::_strnicmp(a, b, size);
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

    template<>
    const char *StringHelper<char>::strstr(const char *haystack, const char *needle)
    {
        return ::strstr(haystack, needle);
    }


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
            if (*a != *b) return *a - *b;
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
        return 0;
    }

    template<>
    char16_t *StringHelper<char16_t>::strcpy(char16_t *dest, const char16_t *source)
    {
        char16_t *result = dest;
        while (*source) *dest++ = *source++;
        *dest = 0;
        return result;
    }

    template<>
    char16_t *StringHelper<char16_t>::strncpy(char16_t *dest, const char16_t *source, size_t n)
    {
        char16_t *result = dest;
        while (*source && n--) *dest++ = *source++;
        return result;
    }

    template<>
    const char16_t *StringHelper<char16_t>::strstr(const char16_t *haystack, const char16_t *needle)
    {
        if (!haystack || !needle)
            return nullptr;

        while (*haystack)
        {
            bool match = true;
            for (const char16_t *p = needle, *p1 = haystack; *p && *p1; ++p, ++p1)
            {
                if (*p != *p1)
                {
                    match = false;
                    break;
                }
            }
            if (match)
                return haystack;
            ++haystack;
        }
        return nullptr;
    }

} // namespace detail

    template<> const size_t detail::StringData<char>::npos = std::numeric_limits<size_t>::max();
    template<> const size_t StringBase<char>::npos = std::numeric_limits<size_t>::max();

    template<> const size_t detail::StringData<char16_t>::npos = std::numeric_limits<size_t>::max();
    template<> const size_t StringBase<char16_t>::npos = std::numeric_limits<size_t>::max();

    template<>
    StringBase<char> StringBase<char>::ms_empty = "";

    template<>
    StringBase<char16_t> StringBase<char16_t>::ms_empty = U16("");

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
        int resultLength = MultiByteToWideChar(CP_ACP, 0, aString, (int)length, NULL, 0) - 1;
        WString result(nullptr, resultLength);
        MultiByteToWideChar(CP_ACP, 0, aString, (int)length, LPWSTR(result.begin()), resultLength + 1);
        return result;
#else
        WString result;
        iconv_t cd = iconv_open("UTF-16LE", "UTF-8");
        if ((iconv_t)-1 == cd)
        {
            perror("iconv_open(): ");
            return WString();
        }

        size_t inputSize = (size_t)-1 != length ? length : detail::StringHelper<char>::strlen(aString);

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
            perror("iconv_close(): ");
            return WString();
        }
        return result;
#endif
	}

	template<>
    String string_cast<String>(const char16_t *wString, size_t length) {
#ifdef _WIN32
        int resultLength = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wString, (int)length, NULL, 0, NULL, NULL) - 1;
		String result(nullptr, resultLength);
		WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wString, (int)length, const_cast<char *>(result.data()), resultLength + 1, NULL, NULL);
        return result;
#else

        String result;
        iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
        if ((iconv_t)-1 == cd)
        {
            perror("iconv_open(): ");
            return String();
        }

        size_t inputSize = (size_t)-1 != length ? length : detail::StringHelper<char16_t>::strlen(wString);

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
            perror("iconv_close(): ");
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

    template<>
    String string_cast<String>(const std::string& str)
    {
        return str;
    }

    template<>
    String string_cast<String>(const std::basic_string<char16_t>& strW)
    {
        return string_cast<String>(strW.data(), strW.length());
    }

    template<>
    WString string_cast<WString>(const std::string& strA)
    {
        return string_cast<WString>(strA.data(), strA.length());
    }

    template<>
    WString string_cast<WString>(const std::basic_string<char16_t>& str)
    {
        return str;
    }

    template<>
    StringBase<char> StringBase<char>::format(const char *fmt, const VariantArray& args)
    {
        const char *p = fmt;
        char buffer[256];
        size_t i = 0;
        String result;
        while (*p) {
            if (*p == '%') {
                String subformat;
                subformat.reserve(10);
                subformat.append(*p);
                if (*(p + 1) == '%')
                {
                    result.append('%');
                    p++;
                    continue;
                }
                for (;;)
                {
                    EFieldType argType = eFieldVoid;
                    ++p;
                    switch (*p)
                    {
                    case 0:
                        throw std::invalid_argument("Unexpected end of format string");
                    default:
                        throw std::invalid_argument("Invalid character in format string");
                    // flags
                    case '-':
                    case '+':
                    case ' ':
                    case '#':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.':
                    case '*':
                        break;

                    // specifiers
                    case 's':
                        argType = eFieldString;
                        break;
                    case 'c':
                        argType = eFieldInt;
                        break;
                    case 'd':
                    case 'i':
                    case 'u':
                    case 'o':
                    case 'x':
                    case 'X':
                        argType = subformat.contains("ll") ? eFieldUint64 : eFieldUint;
                        break;
                    case 'f':
                    case 'F':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'a':
                    case 'A':
                        argType = eFieldDouble;
                        break;
                    case 'p':
                        argType = eFieldObject;
                        break;
                    }
                    subformat.append(*p);

                    if (argType == eFieldVoid)
                        continue;
                    if (i >= args.size())
                        throw std::invalid_argument("Not enogh arguments for the given format");
                    Variant arg = args[i++];
                    int nChars = -1;

                    switch (argType)
                    {
                    default:
                        throw std::runtime_error("Unhandled argument type");
                    case eFieldString:
                        result.append(variant_cast<String>(arg));
                        nChars = 0;
                        break;
                    case eFieldInt:
                        nChars = snprintf(buffer, sizeof(buffer), subformat.c_str(),
                                                       variant_cast<int32_t>(arg));
                        break;
                    case eFieldUint:
                        nChars = snprintf(buffer, sizeof(buffer), subformat.c_str(),
                                                       variant_cast<uint32_t>(arg));
                        break;
                    case eFieldInt64:
                        nChars = snprintf(buffer, sizeof(buffer), subformat.c_str(),
                                                       variant_cast<int64_t>(arg));
                        break;
                    case eFieldUint64:
                        nChars = snprintf(buffer, sizeof(buffer), subformat.c_str(),
                                                       variant_cast<uint64_t>(arg));
                        break;
                    case eFieldDouble:
                        nChars = snprintf(buffer, sizeof(buffer), subformat.c_str(),
                                                       variant_cast<double>(arg));
                        break;
                    case eFieldObject:
                        nChars = snprintf(buffer, sizeof(buffer), subformat.c_str(),
                                                       arg.buffer());
                        break;
                    }
                    if (nChars < 0)
                        throw std::runtime_error("Generic format error");
                    if (nChars)
                        result.append(buffer, nChars);

                    break;
                }
            } else {
                result.append(*p);
            }
            p++;
        }
        return result;
    }

    template<>
    StringBase<char16_t> StringBase<char16_t>::format(const char16_t *format, const VariantArray& args)
    {
        return string_cast<WString>(String::format(string_cast<String>(format).c_str(), args));
    }

    template<>
    StringBase<char> StringBase<char>::urldecode() const
    {
        static const char xlat[256] = {
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            0, 1, 2, 3, 4, 5, 6, 7,  8, 9,-1,-1,-1,-1,-1,-1,
            -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1
        };
        StringBase<char> result;
        result.reserve(size());
        for (const char *p = data(); *p; ++p) {
            if ((*p == '%') && p[1] && p[2]) {
                const char c1 = xlat[(unsigned)p[1]], c2 = xlat[(unsigned)p[2]];
                if (c1 < 0 || c2 < 0)
                    throw std::invalid_argument("Invalid percent encoded character sequence");
                result.append((c1 << 4) | c2);
                p += 2;
            }
            else if (*p == '+')
                result.append(' ');
            else
                result.append(*p);
        }
        return result;
    }

    template<>
    StringBase<char16_t> StringBase<char16_t>::urldecode() const
    {
        return string_cast<WString>(string_cast<String>(*this).urldecode());
    }

    template<>
    StringBase<char> StringBase<char>::urlencode() const
    {
        OutputStringStream ss;
        ss << std::hex << std::uppercase << std::setfill('0');

        for (const char *p = data(); *p; ++p) {
            if (*p == ' ')
                ss << '+';
            else if (isalnum(*p) || *p == '-' || *p == '_' || *p == '.' || *p == '~')
                ss << *p;
            else {
                ss << '%' << std::setw(2) << (int)*p;
            }
        }
        return ss.str();
    }

    template<>
    StringBase<char16_t> StringBase<char16_t>::urlencode() const
    {
        return string_cast<WString>(string_cast<String>(*this).urlencode());
    }

    std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const String& str)
    {
        os.rdbuf()->sputn(str.data(), str.size());
        return os;
    }

    std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const WString& wstr)
    {
        return os << string_cast<String>(wstr);
    }

    std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, const WString& str)
    {
        os.rdbuf()->sputn(str.data(), str.size());
        return os;
    }

    std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, const String& wstr)
    {
        return os << string_cast<WString>(wstr);
    }

    std::basic_istream<char>& operator>>(std::basic_istream<char>& is, String& str)
    {
        std::string buf;
        is >> buf;
        str = buf;
        return is;
    }

    std::basic_istream<char>& operator>>(std::basic_istream<char>& is, WString& wstr)
    {
        std::string buf;
        is >> buf;
        wstr = string_cast<WString>(buf);
        return is;
    }

    std::basic_istream<char16_t>& operator>>(std::basic_istream<char16_t>& is, WString& wstr)
    {
        std::basic_string<char16_t> buf;
        is >> buf;
        wstr = buf;
        return is;
    }

    std::basic_istream<char16_t>& operator>>(std::basic_istream<char16_t>& is, String& str)
    {
        std::basic_string<char16_t> buf;
        is >> buf;
        str = string_cast<String>(buf);
        return is;
    }
}
