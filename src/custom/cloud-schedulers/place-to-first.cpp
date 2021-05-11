#include "place-to-first.h"

void
sim::custom::FirstAvailableScheduler::UpdateSchedule(
    const SchedulerEvent* scheduler_event)
{
    auto cloud = actor_register_->GetActor<infra::Cloud>(cloud_handle_);
    auto vm_storage =
        actor_register_->GetActor<infra::VMStorage>(cloud->VMStorage());

    for (const auto [vm_uuid, vm_status] : vm_storage->VMs()) {
        if (vm_status != infra::VMStatus::kPendingProvisionVM) {
            continue;
        }

        const auto& vm_ptr = actor_register_->GetActor<infra::VM>(vm_uuid);

        auto first_dc = actor_register_->GetActor<infra::DataCenter>(
            cloud->DataCenters()[0]);
        auto first_server_handle = first_dc->Servers()[0];

        // should be used in a normal scheduler to do checks of capacity
        auto first_server =
            actor_register_->GetActor<infra::Server>(first_server_handle);

        auto server_event = events::MakeInheritedEvent<infra::ServerEvent>(
            first_server_handle, scheduler_event, types::TimeInterval{0});
        server_event->type = infra::ServerEventType::kProvisionVM;
        server_event->vm_uuid = vm_uuid;

        schedule_event(server_event, false);
    }
}
