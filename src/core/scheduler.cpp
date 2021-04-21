#include "scheduler.h"

#include "logger.h"
#include "server.h"

void
sim::core::Scheduler::HandleEvent(const sim::events::Event* event)
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
sim::core::Scheduler::UpdateSchedule(const SchedulerEvent* scheduler_event)
{
    for (const types::UUID uuid : vm_storage_->PendingVMs()) {
        const auto& vm_ptr = vm_storage_->GetVM(uuid);

        auto& server_ptr = cloud_->DataCenters()[0]->Servers()[0];

        auto server_event = new infra::ServerEvent();
        server_event->type = infra::ServerEventType::kProvisionVM;
        server_event->addressee = server_ptr.get();
        server_event->virtual_machine = vm_ptr;
        server_event->happen_time = scheduler_event->happen_time;

        schedule_event(server_event, false);
    }
}
