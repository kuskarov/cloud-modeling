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

    Empty reply;

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
        std::string vm_name;
        uint32_t required_ram{}, cpu_percent{}, io_bandwidth{};

        iss >> vm_name >> required_ram >> cpu_percent >> io_bandwidth;

        if (!required_ram) {
            std::cerr << "Required RAM was not provided\n";
            return;
        }

        if (!cpu_percent) {
            std::cerr << "Required CPU percent was not provided\n";
            return;
        }

        if (!io_bandwidth) {
            std::cerr << "Required IO Bandwidth was not provided\n";
            return;
        }

        CallCreateVM(vm_name, "constant",
                     {{"required_ram", std::to_string(required_ram)},
                      {"required_cpu", std::to_string(cpu_percent)},
                      {"required_bandwidth", std::to_string(io_bandwidth)}});
    } else {
        std::cerr << "Unknown command: " << command << "\n";
    }
}

void
sim::client::SimulatorRPCClient::CallResourceAction(
    ResourceActionType type, std::string_view resource_name)
{
    Empty reply;
    ClientContext cntx{};

    ResourceActionMessage request{};
    request.set_resource_name(resource_name.data(), resource_name.size());
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
                                              std::string_view vm_name)
{
    Empty reply;
    ClientContext cntx{};

    VMActionMessage request{};
    request.set_vm_name(vm_name.data(), vm_name.size());
    request.set_vm_action_type(type);

    auto status = stub_->DoVMAction(&cntx, request, &reply);
    if (status.ok()) {
        CallSimulateAll();
    } else {
        std::cerr << "Remote procedure call failed: " << status.error_message()
                  << "\n";
    }
}

void
sim::client::SimulatorRPCClient::CallCreateVM(
    std::string_view vm_name, std::string_view vm_workload_spec,
    const std::unordered_map<std::string, std::string>& params)
{
    Empty reply;
    ClientContext cntx{};

    CreateVMMessage request{};
    request.set_vm_name(vm_name.data(), vm_name.size());
    request.set_vm_workload_model(vm_workload_spec.data(),
                                  vm_workload_spec.size());

    for (const auto& [key, value] : params) {
        auto kv_ptr = request.add_params();
        kv_ptr->set_key(key);
        kv_ptr->set_value(value);
    }

    auto status = stub_->CreateVM(&cntx, request, &reply);
    if (status.ok()) {
        CallSimulateAll();
    } else {
        std::cerr << "Remote procedure call failed: " << status.error_message()
                  << "\n";
    }
}

void
sim::client::SimulatorRPCClient::CallSimulateAll()
{
    ClientContext cntx{};
    Empty reply;
    LogMessage log_message{};

    std::unique_ptr<ClientReader<LogMessage>> reader(
        stub_->SimulateAll(&cntx, reply));

    while (reader->Read(&log_message)) {
        if (auto it = severity_mapping.find(log_message.severity());
            it != severity_mapping.end()) {
            SimulatorLogger::GetLogger().Log(
                TimeStamp{static_cast<int64_t>(log_message.time())}, it->second,
                log_message.caller_type(), log_message.caller_name(),
                log_message.text());
        } else {
            std::cerr << "Received invalid LogSeverity: "
                      << log_message.severity() << "\n";
        }
    }

    Status status = reader->Finish();
    if (!status.ok()) {
        std::cerr << "Remote procedure call failed: " << status.error_message()
                  << "\n";
    }
}
