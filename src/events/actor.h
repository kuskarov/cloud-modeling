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
class Actor
{
 public:
    explicit Actor(std::string&& type) : type_(type) {}

    virtual void HandleEvent(const Event* event) = 0;

    void SetScheduleFunction(const ScheduleFunction& schedule_function)
    {
        schedule_event = schedule_function;
    }

    void SetOwner(Actor* owner) { owner_ = owner; }

    [[nodiscard]] const std::string& GetName() const { return name_; }
    void SetName(const std::string& name) { name_ = name; }

    [[nodiscard]] const std::string& GetType() const { return type_; }
    void SetType(const std::string& type) { type_ = type; }

    virtual ~Actor() = default;

 protected:
    ScheduleFunction schedule_event;

    std::string type_{"Actor"}, name_{"Unnamed"};

    Actor* owner_{};
};

#define ACTOR_LOG_INFO(...) SimulatorLogger().LogInfo(type_, name_, __VA_ARGS__)
#define ACTOR_LOG_ERROR(...) \
    SimulatorLogger().LogError(type_, name_, __VA_ARGS__)
#define ACTOR_LOG_DEBUG(...) \
    SimulatorLogger().LogDebug(type_, name_, __VA_ARGS__)

}   // namespace sim::events
