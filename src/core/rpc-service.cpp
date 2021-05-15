#include "rpc-service.h"

#include <fmt/core.h>

#include "actor.h"
#include "resource.h"
#include "scheduler.h"
#include "vm-storage.h"

sim::types::UUID
sim::core::SimulatorRPCService::ResolveName(const std::string& name)
{
    types::UUID uuid;
    try {
        uuid = actor_register_->GetActorHandle(name);
        return uuid;
    } catch (...) {
        return types::UUID{};
    }
}

grpc::Status
sim::core::SimulatorRPCService::DoResourceAction(
    ServerContext* context, const ResourceActionMessage* request,
    google::protobuf::Empty* response)
{
    WORLD_LOG_INFO("DoResourceAction() called");

    auto uuid = ResolveName(request->resource_name());

    if (!uuid) {
        return Status{
            StatusCode::INVALID_ARGUMENT,
            fmt::format("Unknown resource name: {}", request->resource_name())};
    }

    auto event = events::MakeEvent<infra::ResourceEvent>(
        uuid, event_loop_->Now(), nullptr);

    switch (request->resource_action_type()) {
        case ResourceActionType::BOOT_RESOURCE_ACTION:
            event->type = infra::ResourceEventType::kBoot;
            break;
        case ResourceActionType::REBOOT_RESOURCE_ACTION:
            event->type = infra::ResourceEventType::kReboot;
            break;
        case ResourceActionType::SHUTDOWN_RESOURCE_ACTION:
            event->type = infra::ResourceEventType::kShutdown;
            break;
        default:
            return Status{StatusCode::INVALID_ARGUMENT,
                          fmt::format("Invalid resource_action_type: {}",
                                      request->resource_action_type())};
    }

    schedule_event(event, false);

    WORLD_LOG_INFO("DoResourceAction() succeeded");

    return Status::OK;
}

grpc::Status
sim::core::SimulatorRPCService::CreateVM(ServerContext* context,
                                         const CreateVMMessage* request,
                                         google::protobuf::Empty* response)
{
    WORLD_LOG_INFO("CreateVM() called");

    types::UUID vm_uuid;
    try {
        auto vm = actor_register_->Make<infra::VM>(request->vm_name());
        vm_uuid = vm->UUID();
        vm->SetWorkload(
            infra::VMWorkload{types::RAMBytes{request->required_ram()}});
    } catch (const std::logic_error& le) {
        return Status{StatusCode::INVALID_ARGUMENT,
                      fmt::format("Name {} is not unique", request->vm_name())};
    }

    auto vm_storage_event = events::MakeEvent<infra::VMStorageEvent>(
        vm_storage_handle_, event_loop_->Now(), nullptr);
    vm_storage_event->type = infra::VMStorageEventType::kVMCreated;
    vm_storage_event->vm_uuid = vm_uuid;

    schedule_event(vm_storage_event, false);

    return Status::OK;
}

grpc::Status
sim::core::SimulatorRPCService::DoVMAction(ServerContext* context,
                                           const VMActionMessage* request,
                                           google::protobuf::Empty* response)
{
    WORLD_LOG_INFO("DoVMAction() called");

    auto uuid = ResolveName(request->vm_name());

    if (!uuid) {
        return Status{StatusCode::INVALID_ARGUMENT,
                      fmt::format("Unknown VM name: {}", request->vm_name())};
    }

    // event that should happen after the chain of events ends
    auto vm_storage_notification = new infra::VMStorageEvent();
    vm_storage_notification->type = infra::VMStorageEventType::kVMProvisioned;
    vm_storage_notification->addressee = vm_storage_handle_;
    vm_storage_notification->vm_uuid = uuid;

    switch (request->vm_action_type()) {
        case VMActionType::PROVISION_VM_ACTION: {
            vm_storage_notification->type =
                infra::VMStorageEventType::kVMProvisioned;

            auto vm_storage_event = events::MakeEvent<infra::VMStorageEvent>(
                vm_storage_handle_, event_loop_->Now(), nullptr);
            vm_storage_event->type =
                infra::VMStorageEventType::kVMProvisionRequested;
            vm_storage_event->vm_uuid = uuid;
            schedule_event(vm_storage_event, true);

            auto scheduler_event = events::MakeEvent<SchedulerEvent>(
                scheduler_handle_, event_loop_->Now(), vm_storage_notification);
            scheduler_event->type = SchedulerEventType::kProvisionVM;

            schedule_event(scheduler_event, false);

            break;
        }

        case VMActionType::REBOOT_VM_ACTION:
            return Status{StatusCode::UNIMPLEMENTED,
                          "VM reboot is not implemented yet"};

        case VMActionType::STOP_VM_ACTION: {
            vm_storage_notification->type =
                infra::VMStorageEventType::kVMStopped;

            auto stop_vm_event = events::MakeEvent<infra::VMEvent>(
                uuid, event_loop_->Now(), vm_storage_notification);
            stop_vm_event->type = infra::VMEventType::kStop;
            break;
        }

        case VMActionType::DELETE_VM_ACTION: {
            vm_storage_notification->type =
                infra::VMStorageEventType::kVMDeleted;

            auto delete_vm_event = events::MakeEvent<infra::VMEvent>(
                uuid, event_loop_->Now(), vm_storage_notification);
            delete_vm_event->type = infra::VMEventType::kDelete;
            break;
        }

        default:
            return Status{StatusCode::INVALID_ARGUMENT,
                          fmt::format("Invalid vm_action_type: {}",
                                      request->vm_action_type())};
    }

    return Status::OK;
}

grpc::Status
sim::core::SimulatorRPCService::SimulateAll(
    ServerContext* context, const google::protobuf::Empty* request,
    ServerWriter<LogMessage>* writer)
{
    WORLD_LOG_INFO("SimulateAll() called");

    SimulatorLogger::AddLoggingCallback(
        [&writer, this](types::TimeStamp ts, LogSeverity severity,
                        const std::string& caller_type,
                        const std::string& caller_name,
                        const std::string& text) {
            LogMessage log_message{};

            log_message.set_time(ts);
            log_message.set_severity(severity_mapping.at(severity));
            log_message.set_caller_type(caller_type);
            log_message.set_caller_name(caller_name);
            log_message.set_text(text);

            writer->Write(log_message);
        });

    event_loop_->SimulateAll();

    SimulatorLogger::RemoveLastLoggingCallback();

    WORLD_LOG_INFO("SimulateAll() succeeded");

    return Status::OK;
}
