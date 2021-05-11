#include "scheduler.h"

#include "logger.h"
#include "server.h"

void
sim::core::IScheduler::HandleEvent(const sim::events::Event* event)
{
    try {
        auto sch_event = dynamic_cast<const SchedulerEvent*>(event);
        if (!sch_event) {
            ACTOR_LOG_ERROR("Unknown event");
        }

        if (sch_event->type == SchedulerEventType::kProvisionVM) {
            UpdateSchedule(sch_event);
        } else {
            ACTOR_LOG_ERROR("Unknown event");
        }

    } catch (std::bad_cast& bc) {
        ACTOR_LOG_ERROR("Unknown event");
    }
}

