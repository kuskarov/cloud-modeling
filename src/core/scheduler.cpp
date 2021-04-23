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

void
sim::core::FirstAvailableScheduler::UpdateSchedule(
    const sim::core::SchedulerEvent* scheduler_event)
{
    for (const std::string& vm_name : vm_storage_->PendingVMs()) {
        const auto& vm_ptr = vm_storage_->GetVM(vm_name);

        auto& server_ptr = cloud_->DataCenters()[0]->Servers()[0];

        auto server_event = new infra::ServerEvent();
        server_event->type = infra::ServerEventType::kProvisionVM;
        server_event->addressee = server_ptr.get();
        server_event->vm_name = vm_name;
        server_event->happen_time = scheduler_event->happen_time;
        server_event->notificator = scheduler_event->notificator;

        schedule_event(server_event, false);
    }
}
