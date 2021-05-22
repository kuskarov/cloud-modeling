#pragma once

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/os.h>

#include <ctime>
#include <functional>
#include <string_view>

#include "types.h"

namespace sim {

enum class LogSeverity : int32_t
{
    kError = 0,
    kInfo = 1,
    kDebug = 2,
};

static const char*
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

typedef std::function<void(TimeStamp, LogSeverity, std::string_view,
                           std::string_view, std::string_view)>
    LoggingCallback;

class SimulatorLogger
{
 public:
    static SimulatorLogger& GetLogger()
    {
        static SimulatorLogger logger{};

        return logger;
    }

    void SetCSVFolder(std::string_view path_to_csv_folder)
    {
        csv_file_name_ = std::string{path_to_csv_folder} + "/" +
                         std::to_string(time(nullptr)) + ".csv";
    }

    void SetMaxConsoleSeverity(LogSeverity max_severity)
    {
        max_console_severity = max_severity;
    }

    void SetMaxCSVSeverity(LogSeverity max_severity)
    {
        max_csv_severity = max_severity;
    }

    void SetTimeCallback(NowFunction now_function)
    {
        now = std::move(now_function);
    }

    void PushLoggingCallback(LoggingCallback logging_callback)
    {
        callbacks_.push_back(std::move(logging_callback));
    }

    void PopLoggingCallback() { callbacks_.pop_back(); }

    template <typename... Args>
    void LogNow(LogSeverity severity, std::string_view caller_type,
                std::string_view caller_name, std::string_view format_string,
                Args&&... args)
    {
        Log(now(), severity, caller_type, caller_name, format_string,
            std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Log(TimeStamp ts, LogSeverity severity, std::string_view caller_type,
             std::string_view caller_name, std::string_view format_string,
             Args&&... args)
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
    SimulatorLogger() = default;

    std::string csv_file_name_{};

    NowFunction now{};

    LogSeverity max_console_severity{LogSeverity::kDebug},
        max_csv_severity{LogSeverity::kDebug};

    fmt::ostream& CSVFileStream()
    {
        static fmt::ostream csv_file_stream_ = fmt::output_file(csv_file_name_);

        return csv_file_stream_;
    }

    LoggingCallback GetPrintToConsoleCallback()
    {
        return [this](TimeStamp ts, LogSeverity severity,
                      std::string_view caller_type,
                      std::string_view caller_name, std::string_view text) {
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

    LoggingCallback GetWriteToCSVCallback()
    {
        return [this](TimeStamp ts, LogSeverity severity,
                      std::string_view caller_type,
                      std::string_view caller_name, std::string_view text) {
            if (!csv_file_name_.empty() && severity <= max_csv_severity) {
                CSVFileStream().print("{},{},{},{},{}\n", ts,
                                      SeverityToString(severity), caller_type,
                                      caller_name, text);
            }
        };
    }

    std::vector<LoggingCallback> callbacks_;
};

}   // namespace sim
