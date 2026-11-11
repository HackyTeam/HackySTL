#pragma once

#include <cassert>
#include <cstdio>

// Checks code that ends the process with abort(), such as hsd::panic. The action
// runs in a forked child, so the parent test keeps running after the check.
//
// In coverage builds the child dumps the gcov counters from a SIGABRT handler before
// it dies. Without that, counters written by the aborting code would be lost, because
// abort() skips the normal exit path. Windows has no fork, so the check is skipped
// there. Coverage is measured on Linux, where the check runs.

#if defined(__unix__) || defined(__APPLE__)
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>
#define HSD_TEST_FORK_AVAILABLE 1
#endif

#if defined(HACKYSTL_COVERAGE)
// Defined by libgcov. CMakeLists.txt sets HACKYSTL_COVERAGE when coverage is enabled.
extern "C" void __gcov_dump(void);
#endif

namespace test_support
{
#if defined(HSD_TEST_FORK_AVAILABLE)
    inline void dump_coverage_and_abort(int signal_number)
    {
#if defined(HACKYSTL_COVERAGE)
        __gcov_dump();
#endif
        std::signal(signal_number, SIG_DFL);
        std::raise(signal_number);
    }
#endif

    // Runs `action` in a child process and checks that the child ends with SIGABRT.
    template <typename Action>
    void expect_abort(Action action)
    {
#if defined(HSD_TEST_FORK_AVAILABLE)
        std::fflush(nullptr);
        const pid_t _pid = fork();
        assert(_pid >= 0);

        if (_pid == 0)
        {
            std::signal(SIGABRT, dump_coverage_and_abort);
            action();

            // The action was expected to abort. Returning from it is a failure.
            _exit(0);
        }

        int _status = 0;
        const pid_t _waited = waitpid(_pid, &_status, 0);
        assert(_waited == _pid);
        assert(WIFSIGNALED(_status) && WTERMSIG(_status) == SIGABRT);
#else
        (void)action;
#endif
    }
}
