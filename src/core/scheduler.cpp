#include "scheduler.h"

#include "logger.h"
#include "server.h"

void
sim::core::IScheduler::HandleEvent(const sim::events::Event* event)
{
    auto sch_event = dynamic_cast<const SchedulerEvent*>(event);

    if (!sch_event) {
        ACTOR_LOG_ERROR("Received invalid event");
        return;
    }

    if (sch_event->type == SchedulerEventType::kProvisionVM) {
        UpdateSchedule(sch_event);
    } else {
        ACTOR_LOG_ERROR("Received event with invalid type");
    }
}
