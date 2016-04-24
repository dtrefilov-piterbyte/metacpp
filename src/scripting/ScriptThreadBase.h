#ifndef SCRIPTTHREADBASE_H
#define SCRIPTTHREADBASE_H
#include "config.h"
#include "ScriptProgramBase.h"
#include <functional>

namespace metacpp {
namespace scripting {

#ifdef MSVC
#define _NOEXCEPT
#else
#define _NOEXCEPT noexcept
#endif

/** An exception indicating that the script execution being terminated with
 *  ScriptThreadBase::abort */
class TerminationException : public std::exception
{
public:
    const char *what() const _NOEXCEPT override;
};

/** \brief Basic class providing execution methods and context */
class ScriptThreadBase {
public:
    ScriptThreadBase();
    virtual ~ScriptThreadBase();

    /** \brief Check whether this thread is currently running */
    virtual bool running() const = 0;
    /** \brief If supported, enters state signaling thread termination
     *  and waits for it to finish for the given amount of millisiconds
     * \arg timeout_ms wait timeout in milliseconds, 0 for unlimited wait
     * \returns true if thread successfully finished or terminated or thread is not currently running,
     * false if operations is not supported or timed out
     */
    virtual bool abort(unsigned timeout_ms = 0) = 0;
    /** \brief If script is currently running wait for it to finish for the given amount of time
     * \arg timeout_ms wait timeout in milliseconds, 0 for unlimited wait
     * \returns true if successfully joined with running thread or thread is not running,
     * false if operation is not supported or timed out
     */
    virtual bool wait(unsigned timeout_ms = 0) = 0;

    /** \brief Executes given script thread */
    virtual Variant run() = 0;

    // TODO: Implement worker pool?
    /** \brief Executes given script in a separate newly spawned thread and calls
     * given callbacks upon execution finish or error */
    bool runAsync(const std::function<void(const Variant& result)>& onFinished,
                  const std::function<void(const std::exception_ptr)>& onError);
};

} // namespace scripting
} // namespace metacpp

#endif // SCRIPTTHREADBASE_H
