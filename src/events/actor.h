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
    explicit IActor(std::string&& type)
        : type_(type), uuid_(types::GenerateUUID())
    {
    }

    virtual void HandleEvent(const Event* event) = 0;

    void SetScheduleFunction(const ScheduleFunction& schedule_function)
    {
        schedule_event = schedule_function;
    }

    void SetOwner(types::UUID owner) { owner_ = owner; }

    [[nodiscard]] const std::string& GetName() const { return name_; }
    virtual void SetName(std::string name) { name_ = std::move(name); }

    [[nodiscard]] const std::string& GetType() const { return type_; }
    void SetType(std::string type) { type_ = std::move(type); }

    [[nodiscard]] types::UUID UUID() const { return uuid_; }

    virtual ~IActor() = default;

 protected:
    ScheduleFunction schedule_event;

    std::string type_{"Actor"}, name_{"Unnamed"};

    types::UUID owner_{};

 private:
    const types::UUID uuid_;
};

#define ACTOR_LOG_INFO(...) SimulatorLogger().LogInfo(type_, name_, __VA_ARGS__)
#define ACTOR_LOG_ERROR(...) \
    SimulatorLogger().LogError(type_, name_, __VA_ARGS__)
#define ACTOR_LOG_DEBUG(...) \
    SimulatorLogger().LogDebug(type_, name_, __VA_ARGS__)

}   // namespace sim::events
