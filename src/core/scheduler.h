#pragma once

#include <utility>

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
    IScheduler(std::shared_ptr<infra::Cloud> cloud,
               std::shared_ptr<VMStorage> vm_storage)
        : events::IActor("Scheduler"),
          cloud_(std::move(cloud)),
          vm_storage_(std::move(vm_storage))
    {
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

    // scheduler has read access to Cloud and VMStorage states
    std::shared_ptr<infra::Cloud> cloud_{};
    std::shared_ptr<VMStorage> vm_storage_{};
};

class FirstAvailableScheduler : public IScheduler
{
 public:
    FirstAvailableScheduler(std::shared_ptr<infra::Cloud> cloud,
                            std::shared_ptr<VMStorage> vm_storage)
        : IScheduler(std::move(cloud), std::move(vm_storage))
    {
    }

 protected:
    void UpdateSchedule(const SchedulerEvent* scheduler_event) override;
};

}   // namespace sim::core
