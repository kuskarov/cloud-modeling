#pragma once

#include <utility>

#include "actor-register.h"
#include "actor.h"
#include "cloud.h"
#include "event.h"
#include "vm-storage.h"

namespace sim::core {

/**
 * Abstract class for Scheduler.
 *
 * Scheduler has access to the Cloud state and provides method for new VM
 * placement
 *
 */
class IScheduler : public events::Observer
{
 public:
    IScheduler() : events::Observer("Scheduler") {}

    /**
     * Main schedule method which should be overridden
     *
     * Input --- state of cloud_ and vm_storage_
     * Output --- scheduled events for cloud_ and vm_storage_
     */
    virtual void UpdateSchedule() = 0;
};

}   // namespace sim::core
