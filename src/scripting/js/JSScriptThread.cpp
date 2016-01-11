#include "JSScriptThread.h"
#include "JSScriptEngine.h"
#include "Object.h"
#include <thread>

namespace metacpp
{
namespace scripting
{
namespace js
{

namespace details
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
        size_t length;
        auto pChars = reinterpret_cast<const char16_t *>
                (JS_GetStringCharsZAndLength(context, v.toString(), &length));
        return string_cast<String>(pChars, length);
    }

    if (v.isObject())
    {
        JSObject& obj = v.toObject();
        if (JS_IsArrayObject(context, &obj))
        {
            uint32_t length = 0;
            if (!JS_GetArrayLength(context, &obj, &length))
                throw std::invalid_argument("Could not retrieve array value length");
            VariantArray va;
            va.reserve(length);
            for (uint32_t i = 0; i < length; ++i)
            {
                JS::Value subval;
                if (!JS_GetElement(context, &obj, i, &subval))
                    throw std::invalid_argument("Could not get array element");
                va.push_back(fromValue(context, subval));
            }
            return va;
        }
        //JSClass *cl =JS_GetClass(&obj);
        //return Variant();
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
        auto values = a.map<jsval>([context](const Variant& v) { return toValue(context, v); });
        value.setObject(*JS_NewArrayObject(context, values.size(), values.data()));
        return value;
    }

    if (v.isObject())
    {

    }

    throw std::invalid_argument("Could not convert variant to JS value");
}
} // namespace details

JSScriptThread::JSScriptThread(JSRuntime *parentRuntime, const ByteArray &bytecode,
                               const JSClass *global_class)
    : m_runtime(nullptr), m_context(nullptr), m_script(nullptr), m_global(nullptr),
      m_bRunning(false), m_bTerminating(false)
{
    (void)parentRuntime;
    m_runtime = JS_NewRuntime(8 * 1024 * 1024, JS_USE_HELPER_THREADS);
    if (!m_runtime)
        throw std::runtime_error("Could not create JS runtime");
    JS_SetRuntimePrivate(m_runtime, this);
    m_context = JS_NewContext(m_runtime, 8192);
    if (!m_context)
        throw std::runtime_error("Could not create context");
    m_global = JS_NewGlobalObject(m_context, const_cast<JSClass *>(global_class), nullptr);
    JS_EnterCompartment(m_context, m_global);
    JS_InitStandardClasses(m_context, m_global);
    m_script = JS_DecodeScript(m_context, bytecode.data(), bytecode.size(), nullptr, nullptr);
    if (!m_script)
        throw std::runtime_error("Could not decode compiled script");
    JS_SetErrorReporter(m_context, JSScriptThread::dispatchError);
    JS_SetOperationCallback(m_context, JSScriptThread::operationCallback);
}

JSScriptThread::~JSScriptThread()
{
    if (m_context) {
        JS_LeaveCompartment(m_context, nullptr);
        JS_DestroyContext(m_context);
    }
    if (m_runtime) {
        JS_DestroyRuntime(m_runtime);
    }
}

void JSScriptThread::setCallFunction(const String &functionName, const VariantArray &args)
{
    m_functionName = functionName;
    m_arguments = args;
}

bool JSScriptThread::running() const
{
    return false;
}

Variant JSScriptThread::run()
{
    if (!m_script)
        throw std::runtime_error("Invalid script");

    JS_SetRuntimeThread(m_runtime);
    m_exception = nullptr;

    JS::Value value;

    m_bRunning = true;
    bool bRes = JS_ExecuteScript(m_context, m_global, m_script, &value);

    if (bRes && !m_functionName.isNullOrEmpty()) {
        auto values = m_arguments.map<JS::Value>([this](const Variant& v) {
            return details::toValue(m_context, v);
        });
        bRes = JS_CallFunctionName(m_context, m_global, m_functionName.c_str(), values.size(),
                                   values.data(), &value);
    }
    m_bRunning = false;
    if (!bRes)
    {
        if (m_exception)
            std::rethrow_exception(m_exception);
        else
            throw std::runtime_error("Execution failed");
    }

    return details::fromValue(m_context, value);
}

bool JSScriptThread::abort()
{
    m_bTerminating = true;
    JS_TriggerOperationCallback(m_runtime);
    uint32_t ctr = 0;
    while (m_bRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (ctr++ >= 50)
            return false;
    }
    return true;
}

void JSScriptThread::onError(const char *message, JSErrorReport *report)
{
    m_exception = std::make_exception_ptr(
                ScriptRuntimeError(message, report->filename, report->lineno, report->column));
}

void JSScriptThread::dispatchError(JSContext *ctx, const char *message, JSErrorReport *report)
{
    JSScriptThread *thread = reinterpret_cast<JSScriptThread *>
            (JS_GetRuntimePrivate(JS_GetRuntime(ctx)));
    thread->onError(message, report);
}

JSBool JSScriptThread::operationCallback(JSContext *cx)
{
    JSRuntime *runtime = JS_GetRuntime(cx);
    JSScriptThread *thread = reinterpret_cast<JSScriptThread *>
            (JS_GetRuntimePrivate(runtime));
    if (thread->m_bTerminating) {
        thread->m_exception = std::make_exception_ptr(TerminationException());
        return JS_FALSE;
    }
    return JS_TRUE;
}



} // namespace js
} // namespace scripting
} // namespace metacpp
