#include "rpc-service.h"

#include <fmt/core.h>

#include "actor.h"
#include "custom-code.h"
#include "resource.h"
#include "scheduler.h"
#include "vm-storage.h"

grpc::Status
sim::core::SimulatorRPCService::DoResourceAction(
    ServerContext* context, const ResourceActionMessage* request,
    Empty* response)
{
    try {
        infra::ResourceEventType event_type;

        switch (request->resource_action_type()) {
            case ResourceActionType::BOOT_RESOURCE_ACTION:
                event_type = ResourceEventType::kBoot;
                break;
            case ResourceActionType::REBOOT_RESOURCE_ACTION:
                event_type = ResourceEventType::kReboot;
                break;
            case ResourceActionType::SHUTDOWN_RESOURCE_ACTION:
                event_type = ResourceEventType::kShutdown;
                break;
            default:
                return Status{StatusCode::INVALID_ARGUMENT,
                              fmt::format("Invalid resource_action_type: {}",
                                          request->resource_action_type())};
        }

        world_->DoResourceAction(request->resource_name(), event_type);

    } catch (const std::exception& e) {
        return Status{StatusCode::INVALID_ARGUMENT, e.what()};
    }

    return Status::OK;
}

grpc::Status
sim::core::SimulatorRPCService::CreateVM(ServerContext* context,
                                         const CreateVMMessage* request,
                                         Empty* response)
{
    std::unordered_map<std::string, std::string> params;
    for (const auto& entry : request->params()) {
        params[entry.key()] = entry.value();
    }

    try {
        world_->CreateVM(request->vm_name(), request->vm_workload_model(),
                         params);
    } catch (const std::exception& e) {
        return Status{StatusCode::INVALID_ARGUMENT, e.what()};
    }

    return Status::OK;
}

grpc::Status
sim::core::SimulatorRPCService::DoVMAction(ServerContext* context,
                                           const VMActionMessage* request,
                                           Empty* response)
{
    try {
        infra::VMEventType event_type;

        switch (request->vm_action_type()) {
            case VMActionType::PROVISION_VM_ACTION:
                world_->DoProvisionVM(request->vm_name());
                break;

            case VMActionType::REBOOT_VM_ACTION:
                return Status{StatusCode::UNIMPLEMENTED,
                              "VM reboot is not implemented yet"};

            case VMActionType::STOP_VM_ACTION:
                world_->DoStopVM(request->vm_name());
                break;

            case VMActionType::DELETE_VM_ACTION:
                world_->DoDeleteVM(request->vm_name());
                break;

            default:
                return Status{StatusCode::INVALID_ARGUMENT,
                              fmt::format("Invalid vm_action_type: {}",
                                          request->vm_action_type())};
        }
    } catch (const std::exception& e) {
        return Status{StatusCode::INVALID_ARGUMENT, e.what()};
    }

    return Status::OK;
}

grpc::Status
sim::core::SimulatorRPCService::SimulateAll(ServerContext* context,
                                            const Empty* request,
                                            ServerWriter<LogMessage>* writer)
{
    SimulatorLogger::GetLogger().PushLoggingCallback(
        [&writer, this](
            TimeStamp ts, LogSeverity severity, std::string_view caller_type,
            std::string_view caller_name, std::string_view text) {
            LogMessage log_message{};

            log_message.set_time(ts);
            log_message.set_severity(severity_mapping.at(severity));
            log_message.set_caller_type(caller_type.data(), caller_type.size());
            log_message.set_caller_name(caller_type.data(), caller_type.size());
            log_message.set_text(text.data(), text.size());

            writer->Write(log_message);
        });

    world_->SimulateAll();

    SimulatorLogger::GetLogger().PopLoggingCallback();

    return Status::OK;
}
