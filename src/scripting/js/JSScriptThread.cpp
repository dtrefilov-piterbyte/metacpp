#include "JSScriptThread.h"
#include "JSScriptEngine.h"
#include "Object.h"
#include <jsfriendapi.h>

namespace metacpp
{
namespace scripting
{
namespace js
{

namespace detail
{

Variant fromValue(JSContext *context, const JS::Value& v)
{
    if (v.isNullOrUndefined())
        return Variant();

    if (v.isBoolean())
        return v.toBoolean();

    if (v.isInt32())
        return v.toInt32();

    if (v.isNumber())
        return v.toNumber();

    if (v.isString())
    {
        return String(JSAutoByteString(context, v.toString()).ptr());
    }

    if (v.isObject())
    {
        JS::RootedObject obj(context, v.toObjectOrNull());
        if (JS_IsArrayObject(context, obj))
        {
            uint32_t length = 0;
            if (!JS_GetArrayLength(context, obj, &length))
                throw std::invalid_argument("Could not retrieve array value length");
            VariantArray va;
            va.reserve(length);
            for (uint32_t i = 0; i < length; ++i)
            {
#if MOZJS_MAJOR_VERSION >=31
                JS::RootedValue subval(context);
#else
                jsval subval;
#endif

                if (!JS_GetElement(context, obj, i, &subval))
                    throw std::invalid_argument("Could not get array element");
                va.push_back(fromValue(context, subval));
            }
            return va;
        }

        if (JS_ObjectIsDate(context, obj))
        {
#if MOZJS_MAJOR_VERSION >= 38
            return DateTime(static_cast<time_t>(::js::DateGetMsecSinceEpoch(context, obj) / 1E3 + 0.5));
        #else
            return DateTime(static_cast<time_t>(js_DateGetMsecSinceEpoch(obj) / 1E3 + 0.5));
#endif
        }

        return Variant();
    }

    throw std::invalid_argument("Unknown JS value type");
}

JS::Value toValue(JSContext *context, const Variant& v)
{
    JS::Value value;
    if (!v.valid())
    {
        value.setUndefined();
        return value;
    }

    if (eFieldInt64 == v.type() || eFieldUint64 == v.type() || v.isFloatingPoint())
    {
        value.setNumber(variant_cast<double>(v));
        return value;
    }

    if (eFieldBool == v.type())
    {
        value.setBoolean(variant_cast<bool>(v));
        return value;
    }

    if (v.isIntegral())
    {
        value.setInt32(variant_cast<int>(v));
        return value;
    }

    if (v.isString())
    {
        String s = variant_cast<String>(v);
        value.setString(JS_NewStringCopyN(context, s.data(), s.length()));
        return value;
    }

    if (v.isArray())
    {
        VariantArray a = variant_cast<VariantArray>(v);
        JS::AutoValueVector values(context);
        values.reserve(a.size());
        for (const Variant& subval : a)
            values.append(toValue(context, subval));
#if MOZJS_MAJOR_VERSION >= 31
        value.setObject(*JS_NewArrayObject(context, values));
#else
        value.setObject(*JS_NewArrayObject(context, static_cast<int>(values.length()),
                                           values.begin()));
#endif
        return value;
    }

    if (v.isDateTime())
    {
        auto dt = variant_cast<DateTime>(v);
        value.setObject(*JS_NewDateObject(context, dt.year(), static_cast<int>(dt.month()),
                                          dt.day(), dt.hours(), dt.minutes(), dt.seconds()));
        return value;
    }

    if (v.isObject())
    {

    }

    throw std::invalid_argument("Could not convert variant to JS value");
}

} // namespace details

JSScriptThread::JSScriptThread(const ByteArray &bytecode,
                               JSScriptEngine *engine)
    : m_bytecode(bytecode), m_engine(engine),
      m_runtime(nullptr), m_bRunning(false), m_bTerminating(false)
{
}

JSScriptThread::~JSScriptThread()
{
}

void JSScriptThread::setCallFunction(const String &functionName, const VariantArray &args)
{
    m_functionName = functionName;
    m_arguments = args;
}

bool JSScriptThread::running() const
{
    return m_bRunning;
}

Variant JSScriptThread::run()
{
    // SpiderMonkey is currently limiting to exactly one JSRuntime per thread
    // We will reuse root runtime if we are executing in main thread
    std::unique_ptr<JSRuntime, std::function<void(JSRuntime *)> > rt
    {
        m_engine->isInMainThread() ? m_engine->rootRuntime() :
#if MOZJS_MAJOR_VERSION >= 38
            JS_NewRuntime(JS::DefaultHeapMaxBytes, JS::DefaultNurseryBytes,
                m_engine->rootRuntime()),
#else
            JS_NewRuntime(32 * 1024 * 1024, JS_USE_HELPER_THREADS),
#endif
        [this](JSRuntime *r) {
            if (r && !m_engine->isInMainThread()) {
                JS_DestroyRuntime(r);
            }
        }
    };

    if (!rt)
        throw std::runtime_error("Could not initialize JS runtime");

    std::unique_ptr<JSContext, std::function<void(JSContext *)> > cx
    { JS_NewContext(rt.get(), 8192),
                [](JSContext *c) { if (c) JS_DestroyContext(c); } };
    if (!cx)
        throw std::runtime_error("Could not create JS context");

#if MOZJS_MAJOR_VERSION >= 31
    JS::RootedObject global(cx.get(), JS_NewGlobalObject(cx.get(), m_engine->globalClass(), nullptr,
        JS::FireOnNewGlobalHook));
#else
    JS::RootedObject global(cx.get(), JS_NewGlobalObject(cx.get(),
        const_cast<JSClass *>(m_engine->globalClass()), nullptr));
#endif

    JSAutoCompartment ac(cx.get(), global);
    JS_InitStandardClasses(cx.get(), global);
#if MOZJS_MAJOR_VERSION == 24
    JS::RootedScript script(cx.get(), JS_DecodeScript(cx.get(), m_bytecode.data(), m_bytecode.size(),
                                                      nullptr, nullptr));
#elif MOZJS_MAJOR_VERSION == 31
    JS::RootedScript script(cx.get(), JS_DecodeScript(cx.get(), m_bytecode.data(), m_bytecode.size(),
                                                      nullptr));
#else
    JS::RootedScript script(cx.get(), JS_DecodeScript(cx.get(), m_bytecode.data(), m_bytecode.size()));
#endif
    if (!script)
        throw std::runtime_error("Could not decode compiled script");
#if MOZJS_MAJOR_VERSION >= 31
    JS_SetInterruptCallback(rt.get(), JSScriptThread::interruptCallback);
#else
    JS_SetOperationCallback(cx.get(), JSScriptThread::operationCallback);
#endif

#if MOZJS_MAJOR_VERSION >= 38
    JS_SetErrorReporter(rt.get(), JSScriptThread::dispatchError);
#else
    JS_SetErrorReporter(cx.get(), JSScriptThread::dispatchError);
#endif

    JS_BeginRequest(cx.get());
    JS_SetRuntimePrivate(rt.get(), this);

    m_runtime = rt.get();
    m_bRunning = true;
    m_exception = nullptr;

#if MOZJS_MAJOR_VERSION >= 31
    JS::RootedValue value(cx.get());
#else
    jsval value;
#endif
    bool bRes = JS_ExecuteScript(cx.get(), global, script, &value);

    if (bRes && !m_functionName.isNullOrEmpty()) {

        JS::AutoValueVector values(cx.get());
        values.reserve(m_arguments.size());
        for (const Variant& subval : m_arguments)
            values.append(detail::toValue(cx.get(), subval));
#if MOZJS_MAJOR_VERSION >= 31
        bRes = JS_CallFunctionName(cx.get(), global, m_functionName.c_str(), values, &value);
#else
        bRes = JS_CallFunctionName(cx.get(), global, m_functionName.c_str(),
                                   static_cast<unsigned>(values.length()), values.begin(), &value);
#endif
    }

    m_bRunning = false;
    m_finishedVariable.notify_all();

    JS_SetRuntimePrivate(rt.get(), nullptr);
    JS_EndRequest(cx.get());

    if (!bRes)
    {
        if (m_exception)
            std::rethrow_exception(m_exception);
        else
            throw std::runtime_error("Execution failed");
    }

    return detail::fromValue(cx.get(), value);
}

bool JSScriptThread::abort()
{
    if (m_bRunning)
    {
        m_bTerminating = true;
#if MOZJS_MAJOR_VERSION >= 31
        JS_RequestInterruptCallback(m_runtime);
#else
        JS_TriggerOperationCallback(m_runtime);
#endif
        std::unique_lock<std::mutex> _guard(m_runMutex);
        m_finishedVariable.wait(_guard, [this](){ return !m_bRunning; });
        m_bTerminating = false;
    }

    return true;
}

void JSScriptThread::onError(const char *message, JSErrorReport *report)
{
    if (m_bTerminating)
        m_exception = std::make_exception_ptr(TerminationException());
    else
        m_exception = std::make_exception_ptr(
                ScriptRuntimeError(message, report->filename, report->lineno, report->column));
}

void JSScriptThread::dispatchError(JSContext *ctx, const char *message, JSErrorReport *report)
{
    JSScriptThread *thread = reinterpret_cast<JSScriptThread *>
            (JS_GetRuntimePrivate(JS_GetRuntime(ctx)));
    thread->onError(message, report);
}

#if MOZJS_MAJOR_VERSION >= 31
bool JSScriptThread::interruptCallback(JSContext *cx)
#else
JSBool JSScriptThread::operationCallback(JSContext *cx)
#endif
{
    JSRuntime *runtime = JS_GetRuntime(cx);
    JSScriptThread *thread = reinterpret_cast<JSScriptThread *>
            (JS_GetRuntimePrivate(runtime));
    if (thread->m_bTerminating) {
        thread->m_exception = std::make_exception_ptr(TerminationException());
        return false;
    }
    return true;
}



} // namespace js
} // namespace scripting
} // namespace metacpp
