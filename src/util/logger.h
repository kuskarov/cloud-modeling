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
            abort();
    }
}

typedef std::function<void(types::TimeStamp, LogSeverity, const std::string&,
                           const std::string&, const std::string&)>
    LoggingCallback;

class SimulatorLogger
{
 public:
    static void SetCSVFolder(const std::string& path_to_csv_folder)
    {
        csv_file_name_ =
            path_to_csv_folder + "/" + std::to_string(time(nullptr)) + ".csv";
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

    static void AddLoggingCallback(const LoggingCallback& cb)
    {
        callbacks_.push_back(cb);
    }

    static void RemoveLastLoggingCallback() { callbacks_.pop_back(); }

    template <typename... Args>
    static void LogNow(LogSeverity severity, const std::string& caller_type,
                       const std::string& caller_name,
                       const std::string& format_string, Args&&... args)
    {
        Log(now(), severity, caller_type, caller_name, format_string,
            std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void Log(types::TimeStamp ts, LogSeverity severity,
                    const std::string& caller_type,
                    const std::string& caller_name,
                    const std::string& format_string, Args&&... args)
    {
        auto text = fmt::format(format_string, std::forward<Args>(args)...);

        // standard callbacks
        GetPrintToConsoleCallback()(ts, severity, caller_type, caller_name,
                                    text);
        GetWriteToCSVCallback()(ts, severity, caller_type, caller_name, text);

        // additional callbacks, e.g. passing to gRPC stream
        for (const auto& cb : callbacks_) {
            cb(ts, severity, caller_type, caller_name, text);
        }
    }

 private:
    inline static std::string csv_file_name_{};
    inline static std::function<types::TimeStamp()> now{};
    inline static LogSeverity max_console_severity{LogSeverity::kDebug},
        max_csv_severity{LogSeverity::kDebug};

    static fmt::ostream& CSVFileStream()
    {
        static fmt::ostream csv_file_stream_ = fmt::output_file(csv_file_name_);

        return csv_file_stream_;
    }

    static LoggingCallback GetPrintToConsoleCallback()
    {
        return [](types::TimeStamp ts, LogSeverity severity,
                  const std::string& caller_type,
                  const std::string& caller_name, const std::string& text) {
            if (severity <= max_console_severity) {
                if (severity == LogSeverity::kError) {
                    fmt::print(fg(fmt::color::dark_red) | fmt::emphasis::bold,
                               "[{:>5}] [{:>5}] [{:>15}] [{:>25}] {}\n", ts,
                               SeverityToString(severity), caller_type,
                               caller_name, text);
                } else {
                    fmt::print("[{:>5}] [{:>5}] [{:>15}] [{:>25}] {}\n", ts,
                               SeverityToString(severity), caller_type,
                               caller_name, text);
                }
            }
        };
    }

    static LoggingCallback GetWriteToCSVCallback()
    {
        return [](types::TimeStamp ts, LogSeverity severity,
                  const std::string& caller_type,
                  const std::string& caller_name, const std::string& text) {
            if (!csv_file_name_.empty() && severity <= max_csv_severity) {
                CSVFileStream().print("{},{},{},{},{}\n", ts,
                                      SeverityToString(severity), caller_type,
                                      caller_name, text);
            }
        };
    }

    inline static std::vector<LoggingCallback> callbacks_;
};

#define WORLD_LOG_INFO(...) \
    SimulatorLogger::LogNow(LogSeverity::kInfo, "World", whoami_, __VA_ARGS__)
#define WORLD_LOG_ERROR(...) \
    SimulatorLogger::LogNow(LogSeverity::kError, "World", whoami_, __VA_ARGS__)
#define WORLD_LOG_DEBUG(...) \
    SimulatorLogger::Log(LogSeverity::kDebug, "World", whoami_, __VA_ARGS__)

#define ACTOR_LOG_INFO(...) \
    SimulatorLogger::LogNow(LogSeverity::kInfo, type_, name_, __VA_ARGS__)
#define ACTOR_LOG_ERROR(...) \
    SimulatorLogger::LogNow(LogSeverity::kError, type_, name_, __VA_ARGS__)
#define ACTOR_LOG_DEBUG(...) \
    SimulatorLogger::Log(LogSeverity::kDebug, type_, name_, __VA_ARGS__)

}   // namespace sim
