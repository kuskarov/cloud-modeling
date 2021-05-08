#pragma once

#include <utility>

#include "actor-register.h"
#include "actor.h"
#include "cloud.h"
#include "event.h"
#include "vm-storage.h"

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
class IScheduler : public events::IActor
{
 public:
    IScheduler() : events::IActor("Scheduler") {}

    void SetCloud(types::UUID cloud_handle) { cloud_handle_ = cloud_handle; }

    void SetActorRegister(const ActorRegister* actor_register)
    {
        actor_register_ = actor_register;
    }

    void HandleEvent(const events::Event* event) override;

    ~IScheduler() override = default;

 protected:
    /**
     * Main schedule method which should be overridden
     *
     * Input --- state of cloud_ and vm_storage_
     * Output --- scheduled events for cloud_ and vm_storage_
     */
    virtual void UpdateSchedule(const SchedulerEvent* scheduler_event) = 0;

    const ActorRegister* actor_register_{};

    // scheduler has read access to Cloud state
    types::UUID cloud_handle_{};

    std::function<const IActor*(types::UUID)> get_actor_state;
};

class FirstAvailableScheduler : public IScheduler
{
 protected:
    void UpdateSchedule(const SchedulerEvent* scheduler_event) override;
};

}   // namespace sim::core
