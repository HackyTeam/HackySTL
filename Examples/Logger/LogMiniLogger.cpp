// Title: Mini logger
// Level: Intermediate
// Shows: a small logger that tags messages with the call site taken from stack_trace.

#include <Containers/String.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/Logging.hpp>

using namespace hsd::format_literals;

enum class level
{
    info,
    warning,
    error
};

static const char* level_name(level lvl)
{
    switch (lvl)
    {
        case level::info: return "INFO";
        case level::warning: return "WARN";
        default: return "ERROR";
    }
}

// Collects formatted lines and only keeps messages at or above a threshold.
class mini_logger
{
private:
    hsd::vector<hsd::string> _lines;
    level _threshold;

public:
    explicit mini_logger(level threshold)
        : _threshold{threshold}
    {}

    // The default argument is evaluated at the caller, so add() records the
    // function that made the log call rather than log() itself.
    void log(level lvl, const char* message,
        hsd::stack_trace site = hsd::stack_trace::add())
    {
        if (static_cast<int>(lvl) < static_cast<int>(_threshold))
        {
            return;
        }

        hsd::string line = hsd::string{level_name(lvl)};
        line += hsd::string{" #"};
        line += hsd::to_string(_lines.size());
        line += hsd::string{" "};
        line += hsd::string{site.get().function_name()};
        line += hsd::string{": "};
        line += hsd::string{message};
        _lines.push_back(hsd::move(line));
    }

    void dump(const char* title) const
    {
        hsd::println("{} ({} lines)"_fmt, title, _lines.size());
        for (hsd::usize i = 0; i < _lines.size(); ++i)
        {
            hsd::println("  {}"_fmt, _lines[i]);
        }
    }
};

static void connect(mini_logger& logger)
{
    logger.log(level::info, "connecting");
    logger.log(level::warning, "slow handshake");
}

int main()
{
    mini_logger everything{level::info};
    mini_logger quiet{level::warning};

    everything.log(level::info, "starting");
    connect(everything);
    everything.log(level::error, "gave up");
    everything.dump("everything");

    quiet.log(level::info, "starting");
    connect(quiet);
    quiet.log(level::error, "gave up");
    quiet.dump("quiet");

    return 0;
}
