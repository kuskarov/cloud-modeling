#include "rpc-service.h"

#include <fmt/core.h>

#include "actor.h"
#include "custom-code.h"
#include "resource.h"
#include "scheduler.h"
#include "vm-storage.h"

using namespace sim::infra;

sim::UUID
sim::core::SimulatorRPCService::ResolveName(const std::string& name)
{
    UUID uuid;
    try {
        uuid = actor_register_->GetActorHandle(name);
        return uuid;
    } catch (...) {
        return UUID{};
    }
}

grpc::Status
sim::core::SimulatorRPCService::DoResourceAction(
    ServerContext* context, const ResourceActionMessage* request,
    Empty* response)
{
    WORLD_LOG_INFO("DoResourceAction() called");

    auto resource_uuid = ResolveName(request->resource_name());

    if (!resource_uuid) {
        return Status{
            StatusCode::INVALID_ARGUMENT,
            fmt::format("Unknown resource name: {}", request->resource_name())};
    }

    auto event = events::MakeEvent<ResourceEvent>(resource_uuid,
                                                  event_loop_->Now(), nullptr);

    switch (request->resource_action_type()) {
        case ResourceActionType::BOOT_RESOURCE_ACTION:
            event->type = ResourceEventType::kBoot;
            break;
        case ResourceActionType::REBOOT_RESOURCE_ACTION:
            event->type = ResourceEventType::kReboot;
            break;
        case ResourceActionType::SHUTDOWN_RESOURCE_ACTION:
            event->type = ResourceEventType::kShutdown;
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
                                         Empty* response)
{
    WORLD_LOG_INFO("CreateVM() called");

    UUID vm_uuid;
    try {
        auto vm = actor_register_->Make<VM>(request->vm_name());
        vm_uuid = vm->GetUUID();

        auto workload_model =
            custom::GetWorkloadModel(request->vm_workload_model());

        // TODO: redundant memory usage, pass iterators to Setup()
        std::unordered_map<std::string, std::string> params;
        for (const auto& entry : request->params()) {
            params[entry.key()] = entry.value();
        }

        workload_model->Setup(params);
        vm->SetWorkloadModel(workload_model);
    } catch (const std::exception& e) {
        return Status{
            StatusCode::INVALID_ARGUMENT,
            fmt::format("Exception occured while setting up VM: {}", e.what())};
    }

    auto vmst_event = events::MakeEvent<VMStorageEvent>(
        vm_storage_handle_, event_loop_->Now(), nullptr);
    vmst_event->type = VMStorageEventType::kVMCreated;
    vmst_event->vm_uuid = vm_uuid;

    schedule_event(vmst_event, false);

    return Status::OK;
}

grpc::Status
sim::core::SimulatorRPCService::DoVMAction(ServerContext* context,
                                           const VMActionMessage* request,
                                           Empty* response)
{
    WORLD_LOG_INFO("DoVMAction() called");

    auto vm_uuid = ResolveName(request->vm_name());

    if (!vm_uuid) {
        return Status{StatusCode::INVALID_ARGUMENT,
                      fmt::format("Unknown VM name: {}", request->vm_name())};
    }

    // event that should happen after the chain of events ends
    auto vmst_callback = new VMStorageEvent();
    vmst_callback->type = VMStorageEventType::kVMProvisioned;
    vmst_callback->addressee = vm_storage_handle_;
    vmst_callback->vm_uuid = vm_uuid;

    switch (request->vm_action_type()) {
        case VMActionType::PROVISION_VM_ACTION: {
            vmst_callback->type = VMStorageEventType::kVMProvisioned;

            auto vmst_event = events::MakeEvent<VMStorageEvent>(
                vm_storage_handle_, event_loop_->Now(), nullptr);
            vmst_event->type = VMStorageEventType::kVMProvisionRequested;
            vmst_event->vm_uuid = vm_uuid;
            schedule_event(vmst_event, true);

            auto scheduler_event = events::MakeEvent<SchedulerEvent>(
                scheduler_handle_, event_loop_->Now(), vmst_callback);
            scheduler_event->type = SchedulerEventType::kProvisionVM;

            schedule_event(scheduler_event, false);

            break;
        }

        case VMActionType::REBOOT_VM_ACTION:
            return Status{StatusCode::UNIMPLEMENTED,
                          "VM reboot is not implemented yet"};

        case VMActionType::STOP_VM_ACTION: {
            vmst_callback->type = VMStorageEventType::kVMStopped;

            auto stop_vm_event = events::MakeEvent<VMEvent>(
                vm_uuid, event_loop_->Now(), vmst_callback);
            stop_vm_event->type = VMEventType::kStop;

            schedule_event(stop_vm_event, false);

            break;
        }

        case VMActionType::DELETE_VM_ACTION: {
            vmst_callback->type = VMStorageEventType::kVMDeleted;

            auto delete_vm_event = events::MakeEvent<VMEvent>(
                vm_uuid, event_loop_->Now(), vmst_callback);
            delete_vm_event->type = VMEventType::kDelete;

            schedule_event(delete_vm_event, false);

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
sim::core::SimulatorRPCService::SimulateAll(ServerContext* context,
                                            const Empty* request,
                                            ServerWriter<LogMessage>* writer)
{
    WORLD_LOG_INFO("SimulateAll() called");

    SimulatorLogger::AddLoggingCallback(
        [&writer, this](
            TimeStamp ts, LogSeverity severity, const std::string& caller_type,
            const std::string& caller_name, const std::string& text) {
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
