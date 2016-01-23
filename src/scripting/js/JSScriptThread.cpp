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

struct NativeObjectWrapper {
    Object *nativeObject;
    enum {
        OwnershipGC,
        OwnershipVariant
    } ownership;
};

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

        JSScriptThread *thread = JSScriptThread::getRunningInstance(context);
        if (!thread)
            throw std::runtime_error("No script thread running");
        auto cl = JS_GetClass(obj);
        const ClassInfo *classInfo = thread->findRegisteredClass(cl->name);
        if (!classInfo)
            throw std::invalid_argument("Unsupported class");

        auto wrapper = (NativeObjectWrapper *)JS_GetPrivate(obj);
        if (wrapper) {
            wrapper->ownership = NativeObjectWrapper::OwnershipVariant;
            return wrapper->nativeObject;
        }
        else
            return (Object *)nullptr;
    }

    throw std::invalid_argument("Unknown JS value type");
}

JS::Value toValue(JSContext *context, Variant v)
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
        Object *nativeObject = v.extractObject();
        if (nativeObject)
        {
            auto thread = JSScriptThread::getRunningInstance(context);

            auto ci = thread->findRegisteredClass(nativeObject->metaObject()->name());
            if (!ci) {
                throw std::invalid_argument("Class is not registered");
            }

            JS::RootedObject object(context, JS_NewObject(context, &ci->class_));

            detail::NativeObjectWrapper *wrapper = new detail::NativeObjectWrapper;
            wrapper->nativeObject = nativeObject;
            wrapper->ownership = detail::NativeObjectWrapper::OwnershipGC;

            JS_SetPrivate(object, wrapper);

            value.setObject(*object);

            return value;
        }
        else
        {
            value.setNull();
            return value;
        }
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

    {
        std::unique_lock<std::mutex> _guard(m_runMutex);
        bool bExpectedRunning = false;
        if (!m_bRunning.compare_exchange_strong(bExpectedRunning, true))
            throw std::runtime_error("Thread is already running");

        m_runtime = rt.get();
    }

    JS_SetRuntimePrivate(rt.get(), this);
    m_exception = nullptr;

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
    registerNativeClasses(cx.get(), global);

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

    JSAutoRequest request(cx.get());



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

    Variant result = detail::fromValue(cx.get(), value);

    JS_SetRuntimePrivate(rt.get(), nullptr);

    if (!bRes)
    {
        if (m_exception)
            std::rethrow_exception(m_exception);
        else
            throw std::runtime_error("Execution failed");
    }

    return result;
}

bool JSScriptThread::abort(unsigned timeout_ms)
{
    std::unique_lock<std::mutex> _guard(m_runMutex);
    if (m_bRunning)
    {
        m_bTerminating = true;
#if MOZJS_MAJOR_VERSION >= 31
        JS_RequestInterruptCallback(m_runtime);
#else
        JS_TriggerOperationCallback(m_runtime);
#endif

        bool result = true;
        auto predicate = [this]{ return !m_bRunning; };
        if (timeout_ms)
            result = m_finishedVariable.wait_for(_guard, std::chrono::milliseconds(timeout_ms),
                predicate);
        else
            m_finishedVariable.wait(_guard, predicate);

        m_bTerminating = false;
        return result;
    }

    return true;
}

bool JSScriptThread::wait(unsigned timeout_ms)
{
    std::unique_lock<std::mutex> _guard(m_runMutex);
    if (m_bRunning)
    {
        bool result = true;
        auto predicate = [this]{ return !m_bRunning; };
        if (timeout_ms)
            result = m_finishedVariable.wait_for(_guard, std::chrono::milliseconds(timeout_ms),
                predicate);
        else
            m_finishedVariable.wait(_guard, predicate);
        return result;
    }

    return true;

}

JSScriptThread *JSScriptThread::getRunningInstance(JSContext *cx)
{
    JSRuntime *rt = JS_GetRuntime(cx);
    return reinterpret_cast<JSScriptThread *>(JS_GetRuntimePrivate(rt));
}

const detail::ClassInfo *JSScriptThread::findRegisteredClass(const String &name)
{
    auto it = std::find_if(m_registeredClasses.begin(), m_registeredClasses.end(),
        [&](const detail::ClassInfo& ci)
        {
            return ci.class_.name == name;
        });
    if (it == m_registeredClasses.end())
        return nullptr;
    return it;
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
    JSScriptThread *thread = JSScriptThread::getRunningInstance(ctx);
    if (thread)
        thread->onError(message, report);
}

#if MOZJS_MAJOR_VERSION >= 31
bool JSScriptThread::interruptCallback(JSContext *cx)
#else
JSBool JSScriptThread::operationCallback(JSContext *cx)
#endif
{
    JSScriptThread *thread = JSScriptThread::getRunningInstance(cx);
    if (thread && thread->m_bTerminating) {
        thread->m_exception = std::make_exception_ptr(TerminationException());
        return false;
    }
    return true;
}

void JSScriptThread::registerNativeClasses(JSContext *cx, JS::HandleObject global)
{
    for (const MetaObject *mo : m_engine->registeredClasses())
    {

        m_registeredClasses.push_back(detail::ClassInfo());
        detail::ClassInfo& classInfo = m_registeredClasses.back();
        memset(&classInfo.class_, 0, sizeof(JSClass));
        classInfo.metaObject = mo;
        classInfo.class_.name = mo->name();
        classInfo.class_.flags = JSCLASS_HAS_PRIVATE;
        classInfo.class_.getProperty = JSScriptThread::nativeObjectDynamicGetter;
        classInfo.class_.setProperty = JSScriptThread::nativeObjectDynamicSetter;
        classInfo.class_.finalize = JSScriptThread::nativeObjectFinalize;

        JS::RootedObject classObject(cx, JS_InitClass(cx, global, JS::NullPtr(),
            &classInfo.class_, nullptr, 0,
                         nullptr, nullptr, nullptr, nullptr));

        JSFunction *ctorFunc = JS_DefineFunction(cx, global, mo->name(),
            JSScriptThread::nativeObjectConstruct, 1, JSPROP_READONLY | JSPROP_PERMANENT |
                                                 JSPROP_ENUMERATE | JSFUN_CONSTRUCTOR);
        JS::RootedObject ctorObject(cx, JS_GetFunctionObject(ctorFunc));

        JS_LinkConstructorAndPrototype(cx, ctorObject, classObject);

        // define class methods
        for (size_t i = 0; i < mo->totalMethods(); ++i)
        {
            const MetaCallBase *metaCall = mo->method(i);
            if (metaCall->type() == eMethodOwn)
            {
                JS_DefineFunction(cx, classObject, metaCall->name(), &JSScriptThread::nativeObjectOwnMethodCall,
                    metaCall->numArguments(), JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE);
            }
            else if (metaCall->type() == eMethodStatic)
            {
                JS_DefineFunction(cx, ctorObject, metaCall->name(), &JSScriptThread::nativeObjectStaticMethodCall,
                    metaCall->numArguments(), JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE);
            }
        }

        // define class properties
        for (size_t i = 0; i < mo->totalFields(); ++i)
        {
            const MetaFieldBase *field = mo->field(i);
            JS::RootedObject value(cx);
            JS_DefineProperty(cx, classObject, field->name(), value, JSPROP_READONLY | JSPROP_PERMANENT |
                JSPROP_ENUMERATE | JSPROP_SHARED, &JSScriptThread::nativeObjectGetter,
                &JSScriptThread::nativeObjectSetter);
        }
    }
}

void JSScriptThread::nativeObjectFinalize(JSFreeOp *, JSObject *obj)
{
    auto nativeObjectWrapper = (detail::NativeObjectWrapper *)JS_GetPrivate(obj);
    if (nativeObjectWrapper)
    {
        if (nativeObjectWrapper->ownership == detail::NativeObjectWrapper::OwnershipGC)
        {
            Object *nativeObject = nativeObjectWrapper->nativeObject;
            nativeObject->metaObject()->destroyInstance(nativeObject);
        }
        delete nativeObjectWrapper;
    }
}

bool JSScriptThread::nativeObjectConstruct(JSContext *cx, unsigned argc, jsval *vp)
{
    try
    {
        // TODO: custom ctors
        if (argc != 0)
            throw std::invalid_argument("Bad number of arguments in constructor");
        JSScriptThread *thread = JSScriptThread::getRunningInstance(cx);
        if (!thread)
            throw std::runtime_error("No script thread instance currently running");
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

        //if (!args.isConstructing())
        //    throw std::runtime_error("Native should be called as constructor");

        String className = JSAutoByteString(cx,
            JS_GetFunctionId(JS_ValueToFunction(cx, args.calleev()))).ptr();

        const detail::ClassInfo *ci = thread->findRegisteredClass(className);
        if (!ci)
            throw std::runtime_error("Could not find registered class");

        JSObject *obj = JS_NewObject(cx, &ci->class_);

        if (!obj)
            return false;

        Object *pNativeObject = ci->metaObject->createInstance();
        if (pNativeObject == NULL) {
            JS_ReportOutOfMemory(cx);
            return false;
        }
        detail::NativeObjectWrapper *wrapper = new detail::NativeObjectWrapper;
        wrapper->nativeObject = pNativeObject;
        wrapper->ownership = detail::NativeObjectWrapper::OwnershipGC;

        JS_SetPrivate(obj, wrapper);

#if MOZJS_MAJOR_VERSION >= 38
        args.rval().setObject(*obj);
#else
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
#endif
        return true;
    }
    catch (const std::exception& e)
    {
        JS_ReportError(cx, "Native constructor failed: %s", e.what());
        return false;
    }
}

bool JSScriptThread::nativeObjectOwnMethodCall(JSContext *cx, unsigned argc, jsval *vp)
{
    try
    {
        JSScriptThread *thread = JSScriptThread::getRunningInstance(cx);
        if (!thread)
            throw std::runtime_error("No script thread instance currently running");
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        String methodName = JSAutoByteString(cx,
            JS_GetFunctionId(JS_ValueToFunction(cx, args.calleev()))).ptr();
        JS::RootedObject this_(cx, args.thisv().toObjectOrNull());
        auto wrapper = (detail::NativeObjectWrapper *)JS_GetPrivate(this_);
        VariantArray nativeArgs;
        nativeArgs.reserve(args.length());
        for (unsigned i = 0; i < args.length(); ++i)
            nativeArgs.emplace_back(detail::fromValue(cx, args[i]));
        JS::Value result = detail::toValue(cx, wrapper->nativeObject->invoke(methodName, nativeArgs));
        args.rval().set(result);
        return true;
    }
    catch (const std::exception& e)
    {
        JS_ReportError(cx, "Native call failed: %s", e.what());
        return false;
    }
}

bool JSScriptThread::nativeObjectStaticMethodCall(JSContext *cx, unsigned argc, jsval *vp)
{
    try
    {
        JSScriptThread *thread = JSScriptThread::getRunningInstance(cx);
        if (!thread)
            throw std::runtime_error("No script thread instance currently running");
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        String className = JSAutoByteString(cx,
            JS_GetFunctionId(JS_ValueToFunction(cx, args.thisv()))).ptr();
        String methodName = JSAutoByteString(cx,
            JS_GetFunctionId(JS_ValueToFunction(cx, args.calleev()))).ptr();
        auto ci = thread->findRegisteredClass(className);
        if (!ci)
            throw std::runtime_error("Class not registered");
        VariantArray nativeArgs;
        nativeArgs.reserve(args.length());
        for (unsigned i = 0; i < args.length(); ++i)
            nativeArgs.emplace_back(detail::fromValue(cx, args[i]));
        JS::Value result = detail::toValue(cx, ci->metaObject->invoke(methodName, nativeArgs));
        args.rval().set(result);
        return true;

    }
    catch (const std::exception& e)
    {
        JS_ReportError(cx, "Native call failed: %s", e.what());
        return false;
    }
}

bool JSScriptThread::nativeObjectGetter(JSContext *cx, unsigned argc, jsval *vp)
{
    try
    {
        if (argc != 0)
            throw std::runtime_error("Illegal number of arguments for getter");
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        String propName = JSAutoByteString(cx, JS_GetFunctionId(JS_ValueToFunction(cx, args.calleev()))).ptr();
        JS::RootedObject this_(cx, args.thisv().toObjectOrNull());
        auto wrapper = (detail::NativeObjectWrapper *)JS_GetPrivate(this_);
        args.rval().set(detail::toValue(cx, wrapper->nativeObject->getProperty(propName)));
        return true;
    }
    catch (const std::exception& e)
    {
        JS_ReportError(cx, "Native getter failed: %s", e.what());
        return false;
    }
}

bool JSScriptThread::nativeObjectSetter(JSContext *cx, unsigned argc, jsval *vp)
{
    try
    {
        if (argc != 1)
            throw std::runtime_error("Illegal number of arguments for setter");
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        String propName = JSAutoByteString(cx, JS_GetFunctionId(JS_ValueToFunction(cx, args.calleev()))).ptr();
        JS::RootedObject this_(cx, args.thisv().toObjectOrNull());
        auto wrapper = (detail::NativeObjectWrapper *)JS_GetPrivate(this_);
        wrapper->nativeObject->setProperty(propName, detail::fromValue(cx, args[0]));
        return true;
    }
    catch (const std::exception& e)
    {
        JS_ReportError(cx, "Native setter failed: %s", e.what());
        return false;
    }
}

bool JSScriptThread::nativeObjectDynamicGetter(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
    try
    {
        JS::RootedValue idValue(cx);
        if (!JS_IdToValue(cx, id, &idValue))
            throw std::runtime_error("Bad property id");
        String propName = variant_cast<String>(detail::fromValue(cx, idValue));
        auto wrapper = (detail::NativeObjectWrapper *)JS_GetPrivate(obj);
        // method properties
        if (wrapper->nativeObject->metaObject()->methodByName(propName))
            return JS_PropertyStub(cx, obj, id, vp);
        vp.set(detail::toValue(cx, wrapper->nativeObject->getProperty(propName)));
        return true;
    }
    catch (const std::exception& e)
    {
        JS_ReportError(cx, "Native getter failed: %s", e.what());
        return false;
    }
}

bool JSScriptThread::nativeObjectDynamicSetter(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
    try
    {
        (void)strict;
        JS::RootedValue idValue(cx);
        if (!JS_IdToValue(cx, id, &idValue))
            throw std::runtime_error("Bad property id");
        String propName = variant_cast<String>(detail::fromValue(cx, idValue));
        auto wrapper = (detail::NativeObjectWrapper *)JS_GetPrivate(obj);
        wrapper->nativeObject->setProperty(propName, detail::fromValue(cx, vp));
        return true;
    }
    catch (const std::exception& e)
    {
        JS_ReportError(cx, "Native setter failed: %s", e.what());
        return false;
    }
}

} // namespace js
} // namespace scripting
} // namespace metacpp
