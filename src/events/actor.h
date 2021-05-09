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
    explicit IActor(std::string&& type) : type_(type) {}

    virtual void HandleEvent(const Event* event) = 0;

    void SetScheduleFunction(const ScheduleFunction& schedule_function)
    {
        schedule_event = schedule_function;
    }

    void SetOwner(IActor* owner) { owner_ = owner; }

    [[nodiscard]] const std::string& GetName() const { return name_; }
    virtual void SetName(std::string name) { name_ = std::move(name); }

    [[nodiscard]] const std::string& GetType() const { return type_; }
    void SetType(std::string type) { type_ = std::move(type); }

    virtual ~IActor() = default;

 protected:
    ScheduleFunction schedule_event;

    std::string type_{"Actor"}, name_{"Unnamed"};

    IActor* owner_{};
};

#define ACTOR_LOG_INFO(...) \
    SimulatorLogger::Log(type_, name_, LogSeverity::kInfo, __VA_ARGS__)
#define ACTOR_LOG_ERROR(...) \
    SimulatorLogger::Log(type_, name_, LogSeverity::kError, __VA_ARGS__)
#define ACTOR_LOG_DEBUG(...) \
    SimulatorLogger::Log(type_, name_, LogSeverity::kDebug, __VA_ARGS__)

}   // namespace sim::events
