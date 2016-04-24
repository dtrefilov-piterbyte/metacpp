#include "ScriptThreadBase.h"
#include <future>

namespace metacpp {
namespace scripting {

ScriptThreadBase::ScriptThreadBase() {
}

ScriptThreadBase::~ScriptThreadBase() {
}

bool ScriptThreadBase::runAsync(const std::function<void (const Variant &)> &onFinished, const std::function<void (const std::exception_ptr)> &onError)
{
    std::async(std::launch::async,
        [&]{
        try
        {
            onFinished(this->run());
        }
        catch (...)
        {
            onError(std::current_exception());
        }
    });
    return true;
}

const char *TerminationException::what() const _NOEXCEPT
{
    return "Terminated";
}

} // namespace scripting
} // namespace metacpp
