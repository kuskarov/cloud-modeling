#pragma once

#include "actor-register.h"
#include "actor.h"
#include "event-loop.h"

// generated code
#include "api.grpc.pb.h"
#include "api.pb.h"

namespace sim::core {

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;

using simulator_api::CreateVMMessage;
using simulator_api::LogMessage;
using simulator_api::ResourceActionMessage;
using simulator_api::ResourceActionType;
using simulator_api::Simulator;
using simulator_api::VMActionMessage;
using simulator_api::VMActionType;

class SimulatorRPCService final : public Simulator::Service
{
 public:
    void SetScheduleFunction(const events::ScheduleFunction& cb)
    {
        schedule_event = cb;
    }

    void SetEventLoop(events::EventLoop* event_loop)
    {
        event_loop_ = event_loop;
    }

    void SetActorRegister(ActorRegister* actor_register)
    {
        actor_register_ = actor_register;
    }

    void SetHandles(types::UUID cloud, types::UUID scheduler,
                    types::UUID vm_storage)
    {
        cloud_handle_ = cloud;
        scheduler_handle_ = scheduler;
        vm_storage_handle_ = vm_storage;
    }

 private:
    std::string whoami_{"RPC-Server"};

    Status DoResourceAction(ServerContext* context,
                            const ResourceActionMessage* request,
                            google::protobuf::Empty* response) override;
    Status CreateVM(ServerContext* context, const CreateVMMessage* request,
                    google::protobuf::Empty* response) override;
    Status DoVMAction(ServerContext* context, const VMActionMessage* request,
                      google::protobuf::Empty* response) override;
    // event-loop commands
    Status SimulateAll(ServerContext* context, const google::protobuf::Empty* request,
                       ServerWriter<LogMessage>* writer) override;

    types::UUID ResolveName(const std::string& name);

    events::ScheduleFunction schedule_event;

    types::UUID scheduler_handle_{types::NoneUUID()};

    types::UUID cloud_handle_{types::NoneUUID()};

    types::UUID vm_storage_handle_{types::NoneUUID()};

    ActorRegister* actor_register_{};
    events::EventLoop* event_loop_{};

    std::unordered_map<LogSeverity, simulator_api::LogSeverity>
        severity_mapping{
            {LogSeverity::kError,
             simulator_api::LogSeverity::ERROR_LOG_SEVERITY},
            {LogSeverity::kInfo, simulator_api::LogSeverity::INFO_LOG_SEVERITY},
            {LogSeverity::kDebug,
             simulator_api::LogSeverity::DEBUG_LOG_SEVERITY},
        };
};

}   // namespace sim::core
