#pragma once

#include "scheduler.h"

namespace sim::custom {

using namespace sim::core;

class FirstAvailableScheduler : public IScheduler
{
 protected:
    void UpdateSchedule(const SchedulerEvent* scheduler_event) override;
};

}   // namespace sim::custom
