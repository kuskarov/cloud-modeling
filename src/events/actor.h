#pragma once

#include <functional>
#include <string>
#include <utility>

#include "event.h"
#include "logger.h"
#include "types.h"

namespace sim::events {

typedef std::function<void(Event*, bool)> ScheduleFunction;

/**
 * Abstract class for Actor. Each Actor should be able to HandleEvent (may
 * handle own overridden event) and has a callback for scheduling events
 */
class IActor
{
 public:
    explicit IActor(std::string&& type) : type_(type), uuid_(UUID::Generate())
    {
    }

    /// Actor should provide event handler
    virtual void HandleEvent(const Event* event) = 0;

    /// Actor can schedule events
    void SetScheduleFunction(ScheduleFunction schedule_function)
    {
        schedule_event = std::move(schedule_function);
    }

    /// Actor can get current time
    void SetNowFunction(NowFunction now_function)
    {
        now = std::move(now_function);
    }

    void SetOwner(UUID owner) { owner_ = owner; }

    std::string_view GetName() const { return name_; }
    virtual void SetName(std::string name) { name_ = std::move(name); }

    std::string_view GetType() const { return type_; }
    void SetType(std::string type) { type_ = std::move(type); }

    UUID GetUUID() const { return uuid_; }

    virtual ~IActor() = default;

 protected:
    ScheduleFunction schedule_event;
    NowFunction now;

    std::string type_{"Actor"}, name_{"Unnamed"};

    UUID owner_{};

 private:
    const UUID uuid_;
};

}   // namespace sim::events

#define ACTOR_LOG_INFO(...)                                               \
    SimulatorLogger::GetLogger().LogNow(LogSeverity::kInfo, type_, name_, \
                                        __VA_ARGS__)
#define ACTOR_LOG_ERROR(...)                                               \
    SimulatorLogger::GetLogger().LogNow(LogSeverity::kError, type_, name_, \
                                        __VA_ARGS__)
#define ACTOR_LOG_DEBUG(...)                                               \
    SimulatorLogger::GetLogger().LogNow(LogSeverity::kDebug, type_, name_, \
                                        __VA_ARGS__)

#define WORLD_LOG_INFO(...)                                                    \
    SimulatorLogger::GetLogger().LogNow(LogSeverity::kInfo, "World", WhoAmI(), \
                                        __VA_ARGS__)
#define WORLD_LOG_ERROR(...)                                          \
    SimulatorLogger::GetLogger().LogNow(LogSeverity::kError, "World", \
                                        WhoAmI(), __VA_ARGS__)
#define WORLD_LOG_DEBUG(...)                                                   \
    SimulatorLogger::GetLogger().LogNow(LogSeverity::kDebug, "World", whoami_, \
                                        __VA_ARGS__)
