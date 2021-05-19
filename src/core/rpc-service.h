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
using proto_log_severity = simulator_api::LogSeverity;

using google::protobuf::Empty;

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

    void SetActorRegister(events::ActorRegister* actor_register)
    {
        actor_register_ = actor_register;
    }

    void SetHandles(UUID cloud, UUID scheduler, UUID vm_storage)
    {
        cloud_handle_ = cloud;
        scheduler_handle_ = scheduler;
        vm_storage_handle_ = vm_storage;
    }

 private:
    std::string whoami_{"RPC-Server"};

    Status DoResourceAction(ServerContext* context,
                            const ResourceActionMessage* request,
                            Empty* response) override;
    Status CreateVM(ServerContext* context, const CreateVMMessage* request,
                    Empty* response) override;
    Status DoVMAction(ServerContext* context, const VMActionMessage* request,
                      Empty* response) override;

    // event-loop commands
    Status SimulateAll(ServerContext* context, const Empty* request,
                       ServerWriter<LogMessage>* writer) override;

    UUID ResolveName(const std::string& name);

    events::ScheduleFunction schedule_event;

    UUID scheduler_handle_{};

    UUID cloud_handle_{};

    UUID vm_storage_handle_{};

    events::ActorRegister* actor_register_{};
    events::EventLoop* event_loop_{};

    std::unordered_map<LogSeverity, simulator_api::LogSeverity>
        severity_mapping{
            {LogSeverity::kError, proto_log_severity::ERROR_LOG_SEVERITY},
            {LogSeverity::kInfo, proto_log_severity::INFO_LOG_SEVERITY},
            {LogSeverity::kDebug, proto_log_severity::DEBUG_LOG_SEVERITY},
        };
};

}   // namespace sim::core
