#ifndef SCRIPTTHREADBASE_H
#define SCRIPTTHREADBASE_H
#include "ScriptProgramBase.h"
#include <chrono>

namespace metacpp {
namespace scripting {

/** An exception indicating that the script execution being terminated with
 *  ScriptThreadBase::abort */
class TerminationException : public std::exception
{
public:
    const char *what() const noexcept override;
};

/** \brief Basic class providing execution methods and context */
class ScriptThreadBase {
public:
    ScriptThreadBase();
    virtual ~ScriptThreadBase();

    /** \brief Check whether this thread is currently running */
    virtual bool running() const = 0;
    /** \brief If supported, enters state signaling thread termination
     *  and waits for it to finish */
    virtual bool abort() = 0;

    /** \brief Executes given script thread */
    virtual Variant run() = 0;
};

} // namespace scripting
} // namespace metacpp

#endif // SCRIPTTHREADBASE_H
