#include "ScriptThreadBase.h"

namespace metacpp {
namespace scripting {

ScriptThreadBase::ScriptThreadBase() {
}

ScriptThreadBase::~ScriptThreadBase() {
}

const char *TerminationException::what() const noexcept
{
    return "Terminated";
}

} // namespace scripting
} // namespace metacpp
