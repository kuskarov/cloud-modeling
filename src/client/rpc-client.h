#pragma once

#include <grpcpp/grpcpp.h>

#include "logger.h"

// generated code
#include "api.grpc.pb.h"
#include "api.pb.h"

namespace sim::client {

using grpc::Channel;
using grpc::ChannelInterface;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using simulator_api::CreateVMMessage;
using simulator_api::LogMessage;
using simulator_api::ResourceActionMessage;
using simulator_api::ResourceActionType;
using simulator_api::Simulator;
using simulator_api::VMActionMessage;
using simulator_api::VMActionType;

using google::protobuf::Empty;

class SimulatorRPCClient
{
 public:
    explicit SimulatorRPCClient(
        const std::shared_ptr<ChannelInterface>& channel)
        : stub_(Simulator::NewStub(channel))
    {
    }

    bool Setup();

    void ProcessInput(const std::string& input);

 private:
    std::unordered_map<std::string, ResourceActionType> resource_action_mapping{
        {"boot", ResourceActionType::BOOT_RESOURCE_ACTION},
        {"shutdown", ResourceActionType::SHUTDOWN_RESOURCE_ACTION},
        {"reboot", ResourceActionType::REBOOT_RESOURCE_ACTION},
    };

    std::unordered_map<std::string, VMActionType> vm_action_mapping{
        {"provision-vm", VMActionType::PROVISION_VM_ACTION},
        {"stop-vm", VMActionType::STOP_VM_ACTION},
        {"delete-vm", VMActionType::DELETE_VM_ACTION},
    };

    std::unordered_map<simulator_api::LogSeverity, LogSeverity>
        severity_mapping{
            {simulator_api::LogSeverity::ERROR_LOG_SEVERITY,
             LogSeverity::kError},
            {simulator_api::LogSeverity::INFO_LOG_SEVERITY, LogSeverity::kInfo},
            {simulator_api::LogSeverity::DEBUG_LOG_SEVERITY,
             LogSeverity::kDebug},
        };

    void CallResourceAction(ResourceActionType type,
                            const std::string& resource_name);

    void CallVMAction(VMActionType type, const std::string& vm_name);

    void CallCreateVM(
        const std::string& vm_name, const std::string& vm_workload_spec,
        const std::unordered_map<std::string, std::string>& params);

    void CallSimulateAll();

    std::unique_ptr<Simulator::Stub> stub_;
};

static inline auto
CreateChannel(const std::string& address)
{
    return grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
}

}   // namespace sim::client
