#pragma once

#include "actor.h"
#include "cloud.h"
#include "event.h"

namespace sim::core {

enum class SchedulerEventType
{
    kNone,
    kProvisionVM,
    // TODO: anything else?
};

struct SchedulerEvent : events::Event
{
    SchedulerEventType type{SchedulerEventType::kNone};
};

/**
 * Abstract class for Scheduler.
 *
 * Scheduler has access to the Cloud state and provides method for new VM
 * placement
 *
 * Scheduler is an Actor
 *
 */
class Scheduler : events::Actor
{
 public:
    Scheduler() : events::Actor("Scheduler") {}

    void HandleEvent(const std::shared_ptr<events::Event>& event) override;

    ~Scheduler() override = default;

 private:
    const std::shared_ptr<infra::Cloud> cloud_{};
};

}   // namespace sim::core
