#pragma once

#include "scheduler.h"

// generated code
#include "scheduler.grpc.pb.h"
#include "scheduler.pb.h"

namespace sim::core {

using grpc::ClientContext;
using grpc::Status;
using simulator_remote_scheduler::CloudStateMessage;
using simulator_remote_scheduler::ResourceState;
using simulator_remote_scheduler::ResourceStateMessage;
using simulator_remote_scheduler::Scheduler;
using simulator_remote_scheduler::VMMappingMessage;

/**
 * RPC Scheduler interface
 */
class RPCScheduler : public IScheduler
{
 public:
    /**
     * Dumps cloud state, sends it via RPC and
     */
    void UpdateSchedule() override
    {
        ClientContext ctx;
        VMMappingMessage reply;

        auto cloud = actor_register_->GetActor<infra::Cloud>(monitored_);

        auto reader = stub_->UpdateSchedule(&ctx, DumpCloud());

        while (reader->Read(&reply)) {
            auto vmst_event = events::MakeEvent<infra::VMStorageEvent>(
                cloud->GetVMStorage(), now(), nullptr);
            vmst_event->type = infra::VMStorageEventType::kVMScheduled;

            UUID vm_uuid = actor_register_->GetActorHandle(reply.vm_name());

            vmst_event->vm_uuid = vm_uuid;

            schedule_event(vmst_event, true);

            auto server_event = events::MakeEvent<infra::ServerEvent>(
                actor_register_->GetActorHandle(reply.server_name()), now(),
                nullptr);

            server_event->type = infra::ServerEventType::kProvisionVM;
            server_event->vm_uuid = vm_uuid;

            schedule_event(server_event, false);
        }

        Status status = reader->Finish();
        if (!status.ok()) {
            std::cerr << "Remote procedure call failed: "
                      << status.error_message() << "\n";
        }
    }

 private:
    CloudStateMessage DumpCloud()
    {
        CloudStateMessage out;

        auto cloud = actor_register_->GetActor<infra::Cloud>(monitored_);

        auto rsm = out.add_resource_states();
        rsm->set_resource_name(cloud->GetName().data(),
                               cloud->GetName().size());
        rsm->set_resource_state(ResourceState::RUNNING_RESOURCE_STATE);

        for (UUID dch : cloud->GetDataCenters()) {
            auto dc = actor_register_->GetActor<infra::DataCenter>(dch);

            auto rsm = out.add_resource_states();
            rsm->set_resource_name(dc->GetName().data(), dc->GetName().size());
            rsm->set_resource_state(ResourceState::RUNNING_RESOURCE_STATE);

            for (UUID sh : dc->GetServers()) {
                auto server = actor_register_->GetActor<infra::Server>(sh);

                auto rsm = out.add_resource_states();
                rsm->set_resource_name(server->GetName().data(),
                                       server->GetName().size());
                rsm->set_resource_state(ResourceState::RUNNING_RESOURCE_STATE);

                for (UUID vmh : server->GetVMs()) {
                    auto vm = actor_register_->GetActor<infra::VM>(vmh);

                    auto vmm = out.add_vm_mapping();
                    vmm->set_vm_name(vm->GetName().data(),
                                     vm->GetName().size());
                    vmm->set_server_name(server->GetName().data(),
                                         server->GetName().size());
                }
            }
        }

        return out;
    }

    std::unique_ptr<Scheduler::Stub> stub_;
};

}   // namespace sim::core
