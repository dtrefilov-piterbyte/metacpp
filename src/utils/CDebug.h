#ifndef CDEBUG_H
#define CDEBUG_H
#include <string>
#include <sstream>
#include <mutex>
#include <list>
#include <memory>
#include <boost/optional.hpp>

enum eLogLevel {
    _LOG_DEBUG,
    _LOG_WARN,
    _LOG_ERROR,
    _LOG_FATAL
};

class CDebug
{
public:
    CDebug(eLogLevel level = _LOG_DEBUG);
    CDebug(const CDebug& proto);
    CDebug(const char *fmt, ...);
    CDebug(eLogLevel level, const char *fmt, ...);
    ~CDebug();

    template<typename T>
    CDebug& operator<<(const boost::optional<T>& rhs) {
        rhs ? (m_strbuf << *rhs) : (m_strbuf << "(null)");
        return *this;
    }

    template<typename T>
    CDebug& operator <<(const T& v) { m_strbuf << v; return *this; }

private:
    std::ostringstream m_strbuf;
    static std::mutex ms_dbgmut;
    eLogLevel m_level;
    static std::list<std::unique_ptr<class LogConnector> > ms_logConnectors;

    friend class LogInitializer;
};

inline CDebug cdebug() { return CDebug(_LOG_DEBUG); }
inline CDebug cwarning() { return CDebug(_LOG_WARN); }
inline CDebug cerror() { return CDebug(_LOG_ERROR); }
inline CDebug cfatal() { return CDebug(_LOG_FATAL); }

#define debug(fmt, ...) CDebug(_LOG_DEBUG, fmt, __VA_ARGS__)
#define warning(fmt, ...) CDebug(_LOG_WARN, fmt, __VA_ARGS__)
#define error(fmt, ...) CDebug(_LOG_ERROR, fmt, __VA_ARGS__)
#define fatal(fmt, ...) CDebug(_LOG_FATAL, fmt, __VA_ARGS__)

#endif // CDEBUG_H
