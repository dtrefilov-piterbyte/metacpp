#include "CDebug.h"
#include <ctime>
#include <cstdio>
#include <cstdarg>

class LogConnector {
public:
    virtual void open_log() { }
    virtual void print_log(eLogLevel level, const char *logentry) const = 0;
    virtual void close_log() { }
    virtual ~LogConnector() { }
protected:
    const char *prefmt(eLogLevel ll) const {
        switch (ll) {
        case _LOG_DEBUG: return "(INFO)";
        case _LOG_WARN:  return "(WARN)";
        case _LOG_ERROR: return "(ERRO)";
        case _LOG_FATAL: return "(FATA)";
        }
        return "";
    }
};

class EmptyLogConnector : public LogConnector {
    void print_log(eLogLevel, const char *) const override { }
};

class StdoutLogConnector : public LogConnector {
public:
    void print_log(eLogLevel level, const char *logentry) const override {
        time_t t = time(0);
        char buf[0x100];
        strftime(buf, sizeof(buf), "%Y/%m/%d %X", localtime(&t));
        printf("%s %s %s\n", buf, prefmt(level), logentry);
    }
};

#ifdef __unix__
#include <sys/syslog.h>
class SyslogLogConnector : public LogConnector {
public:
    SyslogLogConnector(const char *app_name) : m_app_name(app_name) { }

    void open_log() override {
        openlog(m_app_name.c_str(), LOG_CONS, LOG_USER);

    }

    void print_log(eLogLevel level, const char *logentry) const override {
        int priority = LOG_DEBUG;
        switch (priority) {
        case _LOG_DEBUG: priority = LOG_DEBUG; break;
        case _LOG_WARN:  priority = LOG_WARNING; break;
        case _LOG_ERROR: priority = LOG_ERR; break;
        case _LOG_FATAL: priority = LOG_ALERT; break;
        }
        syslog(priority, "%s %s", prefmt(level), logentry);
    }

    void close_log() override {
        closelog();
    }
private:
    const std::string m_app_name;
};
#endif

#ifdef __ANDROID__
/* TODO: untested, even for compile-time errors */
#include <android/log.h>
class AndroidLogConnector : public LogConnector {
public:
    AndroidLogConnector(const char *app_name) : m_app_name(app_name) { }

    void print_log(eLogLevel level, const char *logentry) const override {
        int priority = LOG_DEBUG;
        switch (priority) {
        case _LOG_DEBUG: priority = ANDROID_LOG_DEBUG; break;
        case _LOG_WARN:  priority = ANDROID_LOG_WARN; break;
        case _LOG_ERROR: priority = ANDROID_LOG_ERROR; break;
        case _LOG_FATAL: priority = ANDROID_LOG_FATAL; break;
        }
        __android_log_write(priority, m_app_name, logentry);
    }

private:
    const std::string m_app_name;
};

#endif

CDebug::CDebug(eLogLevel level)
    : m_level(level)
{
    ms_dbgmut.lock();
}

CDebug::CDebug(const CDebug &proto)
    : m_level(proto.m_level)
{
    /* a lock is already acquired in proto instance */
    m_strbuf << proto.m_strbuf.str();
}

CDebug::CDebug(const char *format, ...)
    : CDebug()
{
    char buf[0x400];
    va_list args;
    va_start(args, format);
    vsprintf(buf,format, args);
    va_end(args);
    m_strbuf << buf;
}

CDebug::CDebug(eLogLevel level, const char *format, ...)
    : CDebug(level)
{
    char buf[0x400];
    va_list args;
    va_start(args, format);
    vsprintf(buf,format, args);
    va_end(args);
    m_strbuf << buf;
}

CDebug::~CDebug()
{
    if (m_strbuf.str().size()) {
        for (auto& connector : ms_logConnectors)
            connector->print_log(m_level, m_strbuf.str().c_str());
        ms_dbgmut.unlock();
    }
}

std::mutex CDebug::ms_dbgmut;
std::list<std::unique_ptr<LogConnector> > CDebug::ms_logConnectors;

class LogInitializer
{
public:
    LogInitializer()
    {
#ifdef __unix__
        // syslog
        {
            const char *syslogName = getenv("CDEBUG_LOG_TO_SYSLOG");
            if (syslogName)
            {
                std::unique_ptr<LogConnector> connector(new SyslogLogConnector(syslogName));
                connector->open_log();
                CDebug::ms_logConnectors.push_back(std::move(connector));
            }
        }
#endif
        // file
        {
            const char *logFileName = getenv("CDEBUG_LOG_TO_FILE");
            // TODO: file log connector
        }

        // stdout
        {
            std::unique_ptr<LogConnector> connector(new StdoutLogConnector());
            connector->open_log();
            CDebug::ms_logConnectors.push_back(std::move(connector));
        }
    }

    ~LogInitializer() {

    }

};

namespace
{
    LogInitializer g_logInitializer;
}
