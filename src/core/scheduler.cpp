#include "scheduler.h"

void
sim::core::Scheduler::HandleEvent(const std::shared_ptr<events::Event>& event)
{
    try {
        auto sch_event = dynamic_cast<const SchedulerEvent*>(event.get());
        if (!sch_event) {
        }

        if (sch_event->type == SchedulerEventType::kProvisionVM) {
        } else {
        }

    } catch (std::bad_cast& bc) {
    }
}
