#pragma once

#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>

#include "types.h"

namespace sim {

// TODO: to be reworked once more (but with no changes to external interface)

static constexpr char delimiter_ = '|';
static constexpr char spaces[] = "                              ";

template <typename It>
static void
align_right(It it_from, It it_to, uint32_t max_length,
            spdlog::memory_buf_t& dest)
{
    if (it_to - it_from >= max_length) {
        it_to = it_from + max_length;
    } else {
        dest.append(spaces, spaces + (max_length - (it_to - it_from)));
    }

    dest.append(it_from, it_to);
}

class DiscreteTimeFormatter : public spdlog::custom_flag_formatter
{
 public:
    void format(const spdlog::details::log_msg& msg, const std::tm& tm,
                spdlog::memory_buf_t& dest) override
    {
        // time|actor-type-name|message

        align_right(
            msg.payload.begin(),
            std::find(msg.payload.begin(), msg.payload.end(), delimiter_), 3,
            dest);
    }

    [[nodiscard]] std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<DiscreteTimeFormatter>();
    }
};

class ActorTypeNameFormatter : public spdlog::custom_flag_formatter
{
 public:
    void format(const spdlog::details::log_msg& msg, const std::tm& tm,
                spdlog::memory_buf_t& dest) override
    {
        // time|actor-type-name|message

        auto it_from = std::next(
            std::find(msg.payload.begin(), msg.payload.end(), delimiter_));

        auto it_to = std::find(it_from, msg.payload.end(), delimiter_);

        align_right(it_from, it_to, 30, dest);
    }

    [[nodiscard]] std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<ActorTypeNameFormatter>();
    }
};

class MessageFormatter : public spdlog::custom_flag_formatter
{
 public:
    void format(const spdlog::details::log_msg& msg, const std::tm& tm,
                spdlog::memory_buf_t& dest) override
    {
        // time|actor-type-name|message

        auto it_first = std::next(
            std::find(msg.payload.begin(), msg.payload.end(), delimiter_));

        auto it_from =
            std::next(std::find(it_first, msg.payload.end(), delimiter_));

        auto it_to = msg.payload.end();

        dest.append(it_from, it_to);
    }

    [[nodiscard]] std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<MessageFormatter>();
    }
};

class SimulatorLogger
{
 public:
    static void EnableCSVLogging(std::string csv_file_name)
    {
        csv_file_name_ = std::move(csv_file_name);
    }

    static void SetTimeCallback(
        const std::function<types::TimeStamp()>& now_callback)
    {
        now = now_callback;
    }

 private:
    static auto GetConsoleSink()
    {
        auto console_sink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);

        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->add_flag<DiscreteTimeFormatter>('j')
            .add_flag<ActorTypeNameFormatter>('a')
            .add_flag<MessageFormatter>('v')
            .set_pattern("%^[%-10j] [%7l] [%-30a] %v%$");
        console_sink->set_formatter(std::move(formatter));

        return console_sink;
    }

    static auto GetFileSink()
    {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            csv_file_name_, false);
        file_sink->set_level(spdlog::level::trace);

        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->add_flag<DiscreteTimeFormatter>('j')
            .add_flag<ActorTypeNameFormatter>('a')
            .add_flag<MessageFormatter>('v')
            .set_pattern("%j,%l,%a,%v");
        file_sink->set_formatter(std::move(formatter));

        return file_sink;
    }

    static std::unique_ptr<spdlog::logger> logger_;
    static std::string csv_file_name_;
    static std::function<types::TimeStamp()> now;

    template <typename FormatString, typename... Args>
    inline void Log(spdlog::level::level_enum level,
                    const std::string& actor_type,
                    const std::string& actor_name, const FormatString& fmt,
                    Args&&... args)
    {
        if (actor_name == actor_type) {
            logger_->log(level,
                         std::to_string(now()) + delimiter_ + actor_type +
                             delimiter_ + fmt,
                         std::forward<Args>(args)...);
        } else {
            logger_->log(level,
                         std::to_string(now()) + delimiter_ + actor_type +
                             " :: " + actor_name + delimiter_ + fmt,
                         std::forward<Args>(args)...);
        }
    }

 public:
    static void Setup()
    {
        if (csv_file_name_.empty()) {
            logger_ =
                std::make_unique<spdlog::logger>("SimLogger", GetConsoleSink());
        } else {
            // https://stackoverflow.com/questions/55446935/spdlog-logger-construction-make-unique-doesnt-compile
            logger_ = std::make_unique<spdlog::logger>(
                "SimLogger",
                spdlog::sinks_init_list{GetConsoleSink(), GetFileSink()});
            logger_->flush_on(spdlog::level::level_enum::trace);
        }
    }

    template <typename FormatString, typename... Args>
    inline void LogInfo(const std::string& actor_type,
                        const std::string& actor_name, const FormatString& fmt,
                        Args&&... args)
    {
        Log(spdlog::level::level_enum::info, actor_type, actor_name, fmt,
            std::forward<Args>(args)...);
    }

    template <typename FormatString, typename... Args>
    inline void LogError(const std::string& actor_type,
                         const std::string& actor_name, const FormatString& fmt,
                         Args&&... args)
    {
        Log(spdlog::level::level_enum::err, actor_type, actor_name, fmt,
            std::forward<Args>(args)...);
    }

    template <typename FormatString, typename... Args>
    inline void LogDebug(const std::string& actor_type,
                         const std::string& actor_name, const FormatString& fmt,
                         Args&&... args)
    {
        Log(spdlog::level::level_enum::debug, actor_type, actor_name, fmt,
            std::forward<Args>(args)...);
    }

    template <typename FormatString, typename... Args>
    inline void LogRawError(const FormatString& fmt, Args&&... args)
    {
        logger_->error(fmt, std::forward<Args>(args)...);
    }
};

}   // namespace sim
