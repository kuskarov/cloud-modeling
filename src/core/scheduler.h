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
class Scheduler : public events::Actor
{
 public:
    Scheduler(std::shared_ptr<infra::Cloud> cloud,
              std::shared_ptr<VMStorage> vm_storage)
        : events::Actor("Scheduler"),
          cloud_(std::move(cloud)),
          vm_storage_(std::move(vm_storage))
    {
    }

    void HandleEvent(const events::Event* event) override;

    ~Scheduler() override = default;

 protected:
    /**
     * Main schedule method which should be overridden
     *
     * Input --- state of cloud_ and vm_storage_
     * Output --- scheduled events for cloud_ and vm_storage_
     */
    virtual void UpdateSchedule(const SchedulerEvent* scheduler_event);

    // scheduler has read access to Cloud and VMStorage states
    std::shared_ptr<infra::Cloud> cloud_{};
    std::shared_ptr<VMStorage> vm_storage_{};
};

}   // namespace sim::core
