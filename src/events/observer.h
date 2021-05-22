#pragma once

#include <utility>

#include "actor-register.h"
#include "actor.h"

namespace sim::events {

/**
 * A base class for instances that are not actors, but have access to the Cloud
 * state and is able to schedule events
 */
class Observer
{
 public:
    explicit Observer(std::string whoami) : whoami_(std::move(whoami)) {}

    const auto& WhoAmI() { return whoami_; }

    void SetActorRegister(const ActorRegister* actor_register)
    {
        actor_register_ = actor_register;
    }

    void SetScheduleFunction(ScheduleFunction schedule_function)
    {
        schedule_event = std::move(schedule_function);
    }

    void SetNowFunction(NowFunction now_function)
    {
        now = std::move(now_function);
    }

    void SetMonitoredActor(UUID monitored) { monitored_ = monitored; }

    virtual ~Observer() = default;

 protected:
    /// Observer can schedule events
    ScheduleFunction schedule_event;

    /// Observer can get current time
    NowFunction now;

    /// Observer can resolve actor UUID to an object using actor register
    const ActorRegister* actor_register_{};

    /// Observer monitors some actor
    UUID monitored_{};

 private:
    const std::string whoami_;
};

}   // namespace sim::events
