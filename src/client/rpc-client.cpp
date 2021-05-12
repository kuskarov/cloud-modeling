#include "rpc-client.h"

#include <sstream>

#include "logger.h"

bool
sim::client::SimulatorRPCClient::Setup()
{
    return true;
}

void
sim::client::SimulatorRPCClient::ProcessInput(const std::string& input)
{
    std::istringstream iss(input);

    std::string command;
    iss >> command;

    CommandStatusMessage reply;

    if (auto it = resource_action_mapping.find(command);
        it != resource_action_mapping.end()) {
        std::string resource_name;
        iss >> resource_name;

        CallResourceAction(it->second, resource_name);
    } else if (auto it2 = vm_action_mapping.find(command);
               it2 != vm_action_mapping.end()) {
        std::string vm_name;
        iss >> vm_name;

        CallVMAction(it2->second, vm_name);
    } else if (command == "create-vm") {
        // temporal! a researcher should provide his custom workload spec
        // parsing manually for now
        std::string vm_name;
        uint32_t required_ram;

        iss >> vm_name >> required_ram;

        if (!required_ram) {
            std::cerr << "Required RAM was not provided\n";
            return;
        }

        VMWorkloadSpec spec{required_ram};

        CallCreateVM(spec, vm_name);
    } else {
        std::cerr << "Unknown command: " << command << "\n";
    }
}

void
sim::client::SimulatorRPCClient::CallResourceAction(
    ResourceActionType type, const std::string& resource_name)
{
    CommandStatusMessage reply{};
    ClientContext cntx{};

    ResourceActionMessage request{};
    request.set_resource_name(resource_name);
    request.set_resource_action_type(type);

    auto status = stub_->DoResourceAction(&cntx, request, &reply);
    if (status.ok()) {
        CallSimulateAll();
    } else {
        std::cerr << "Remote procedure call failed: "
                  << cntx.debug_error_string() << "\n";
    }
}

void
sim::client::SimulatorRPCClient::CallVMAction(VMActionType type,
                                              const std::string& vm_name)
{
    CommandStatusMessage reply{};
    ClientContext cntx{};

    VMActionMessage request{};
    request.set_vm_name(vm_name);
    request.set_vm_action_type(type);

    auto status = stub_->DoVMAction(&cntx, request, &reply);
    if (status.ok()) {
        CallSimulateAll();
    } else {
        std::cerr << "Remote procedure call failed: " << reply.error_text()
                  << "\n";
    }
}

void
sim::client::SimulatorRPCClient::CallCreateVM(
    const VMWorkloadSpec& vm_workload_spec, const std::string& vm_name)
{
    CommandStatusMessage reply{};
    ClientContext cntx{};

    CreateVMMessage request{};
    request.set_vm_name(vm_name);
    request.set_required_ram(vm_workload_spec.required_ram);

    auto status = stub_->CreateVM(&cntx, request, &reply);
    if (status.ok()) {
        CallSimulateAll();
    } else {
        std::cerr << "Remote procedure call failed: " << reply.error_text()
                  << "\n";
    }
}

void
sim::client::SimulatorRPCClient::CallSimulateAll()
{
    ClientContext cntx{};
    EmptyMessage empty_message{};
    LogMessage log_message{};

    std::unique_ptr<ClientReader<LogMessage>> reader(
        stub_->SimulateAll(&cntx, empty_message));

    while (reader->Read(&log_message)) {
        if (auto it = severity_mapping.find(log_message.severity());
            it != severity_mapping.end()) {
            SimulatorLogger::Log(
                types::TimeStamp{static_cast<int64_t>(log_message.time())},
                it->second, log_message.caller_type(),
                log_message.caller_name(), log_message.text());
        } else {
            std::cerr << "Received invalid LogSeverity: "
                      << log_message.severity() << "\n";
        }
    }
    Status status = reader->Finish();
}
