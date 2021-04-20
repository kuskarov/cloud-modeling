#include "scheduler.h"

#include "server.h"

void
sim::core::Scheduler::HandleEvent(const sim::events::Event* event)
{
    try {
        auto sch_event = dynamic_cast<const SchedulerEvent*>(event);
        if (!sch_event) {
            LOG_F(ERROR, "Scheduler: unknown event");
        }

        if (sch_event->type == SchedulerEventType::kProvisionVM) {
            UpdateSchedule(sch_event);
        } else {
            LOG_F(ERROR, "Scheduler: unknown event");
        }

    } catch (std::bad_cast& bc) {
        LOG_F(ERROR, "Scheduler: unknown event");
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
