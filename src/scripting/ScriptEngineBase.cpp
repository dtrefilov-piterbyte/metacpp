#include "ScriptEngineBase.h"

namespace metacpp {
namespace scripting {

ScriptEngineBase::ScriptEngineBase() {
}

ScriptEngineBase::~ScriptEngineBase() {
}

SharedObjectPointer<ScriptProgramBase> ScriptEngineBase::createProgram()
{
    auto deleter = [this](ScriptProgramBase *program) { closeProgramImpl(program); };
    return SharedObjectPointer<ScriptProgramBase>(createProgramImpl(), deleter);
}

void ScriptEngineBase::registerClass(const MetaObject *metaObject)
{
    m_registeredClasses.push_back(metaObject);
}

Array<const MetaObject *> ScriptEngineBase::registeredClasses() const
{
    return m_registeredClasses;
}

ScriptRuntimeError::ScriptRuntimeError(const char *message, const char *filename, uint32_t line, uint32_t column)
{
    m_what = String(filename) + ":" + String::fromValue(line) + ":" + String::fromValue(column) + ":"
            + message;
}

const char *ScriptRuntimeError::what() const _NOEXCEPT
{
    return m_what.c_str();
}

} // namespace scripting
} // namespace metacpp
