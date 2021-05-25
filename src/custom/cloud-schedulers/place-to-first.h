#pragma once

#include "scheduler.h"

namespace sim::custom {

using namespace sim::core;
using namespace sim::infra;
using namespace sim::events;

class FirstAvailableScheduler : public IScheduler
{
 protected:
    void UpdateSchedule() override
    {
        auto cloud = actor_register_->GetActor<Cloud>(monitored_);
        auto vm_storage =
            actor_register_->GetActor<VMStorage>(cloud->GetVMStorage());

        for (const auto [vm_uuid, vm_status] : vm_storage->GetVMs()) {
            if (vm_status != VMStatus::kPending) {
                continue;
            }

            auto vm_ptr = actor_register_->GetActor<VM>(vm_uuid);

            auto first_dc =
                actor_register_->GetActor<DataCenter>(
                cloud->GetDataCenters()[0]);
            auto first_server_handle = first_dc->GetServers()[0];

            // should be used in a normal scheduler to do checks of capacity
            auto first_server =
                actor_register_->GetActor<Server>(first_server_handle);

            auto vmst_event =
                MakeEvent<VMStorageEvent>(cloud->GetVMStorage(), now(), nullptr);
            vmst_event->type = VMStorageEventType::kVMScheduled;
            vmst_event->vm_uuid = vm_uuid;

            schedule_event(vmst_event, true);

            auto server_event =
                MakeEvent<ServerEvent>(first_server_handle, now(), nullptr);
            server_event->type = ServerEventType::kProvisionVM;
            server_event->vm_uuid = vm_uuid;

            schedule_event(server_event, false);
        }
    }
};

}   // namespace sim::custom
