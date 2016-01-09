#include "ScriptEngineBase.h"

namespace metacpp {
namespace scripting {

ScriptEngineBase::ScriptEngineBase() {
}

ScriptEngineBase::~ScriptEngineBase() {
}

std::shared_ptr<ScriptProgramBase> ScriptEngineBase::createProgram()
{
    auto deleter = [this](ScriptProgramBase *program) { closeProgramImpl(program); };
    return std::shared_ptr<ScriptProgramBase>(createProgramImpl(), deleter);
}

ScriptRuntimeError::ScriptRuntimeError(const char *message, const char *filename, uint32_t line, uint32_t column)
{
    m_what = String(filename) + ":" + String::fromValue(line) + ":" + String::fromValue(column) + ":"
            + message;
}

const char *ScriptRuntimeError::what() const noexcept
{
    return m_what.c_str();
}

} // namespace scripting
} // namespace metacpp
