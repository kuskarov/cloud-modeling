#pragma once

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/os.h>

#include <ctime>
#include <functional>

#include "types.h"

namespace sim {

enum class LogSeverity : int32_t
{
    kError = 0,
    kInfo = 1,
    kDebug = 2,
};

inline const char*
SeverityToString(LogSeverity severity)
{
    switch (severity) {
        case LogSeverity::kError:
            return "error";
        case LogSeverity::kInfo:
            return "info";
        case LogSeverity::kDebug:
            return "debug";
        default:
            return "never";
    }
}

class SimulatorLogger
{
 public:
    static void SetCSVFolder(const std::string& path_to_csv_folder)
    {
        csv_file_name_ =
            path_to_csv_folder + "/" + std::to_string(time(nullptr));
    }

    static void SetMaxConsoleSeverity(LogSeverity max_severity)
    {
        max_console_severity = max_severity;
    }

    static void SetMaxCSVSeverity(LogSeverity max_severity)
    {
        max_csv_severity = max_severity;
    }

    static void SetTimeCallback(
        const std::function<types::TimeStamp()>& now_callback)
    {
        now = now_callback;
    }

    template <typename... Args>
    static void Log(const std::string& caller_type,
                    const std::string& caller_name, LogSeverity severity,
                    const std::string& format_string, Args&&... args)
    {
        std::string caller{};
        if (caller_type.empty()) {
            caller = "Core :: " + caller_name;
        } else {
            caller = caller_type + " :: " + caller_name;
        }

        if (severity <= max_console_severity) {
            if (severity == LogSeverity::kError) {
                fmt::print(
                    fg(fmt::color::dark_red) | fmt::emphasis::bold,
                    FMT_STRING("[{:>5}] [{:>5}] [{:>40}] {}\n"), now(),
                    SeverityToString(severity), caller,
                    fmt::format(format_string, std::forward<Args>(args)...));
            } else {
                fmt::print(
                    FMT_STRING("[{:>5}] [{:>5}] [{:>40}] {}\n"), now(),
                    SeverityToString(severity), caller,
                    fmt::format(format_string, std::forward<Args>(args)...));
            }
        }

        if (!csv_file_name_.empty() && severity <= max_csv_severity) {
            CSVFileStream().print(
                FMT_STRING("{},{},{},{}\n"), now(), SeverityToString(severity),
                caller,
                fmt::format(format_string, std::forward<Args>(args)...));
        }
    }

 private:
    inline static std::string csv_file_name_{};
    inline static std::function<types::TimeStamp()> now{};
    inline static LogSeverity max_console_severity{LogSeverity::kDebug},
        max_csv_severity{LogSeverity::kDebug};

    static fmt::ostream& CSVFileStream()
    {
        static fmt::ostream csv_file_stream_ =
            std::move(fmt::output_file(csv_file_name_));

        return csv_file_stream_;
    }
};

}   // namespace sim
