#pragma once

#include "resource-scheduler.h"

namespace sim::custom {

using namespace sim::core;

class ConstantVMWorkloadModel : public infra::IVMWorkloadModel
{
 public:
    void Setup(
        const std::unordered_map<std::string, std::string>& params) override
    {
        if (auto it = params.find("required_ram"); it != params.end()) {
            required_ram_ =
                RAMBytes{static_cast<uint32_t>(std::stoi(it->second))};
        } else {
            throw std::invalid_argument("required_ram field not found");
        }

        if (auto it = params.find("required_cpu"); it != params.end()) {
            required_cpu_ = CPUUtilizationPercent{
                static_cast<uint32_t>(std::stoi(it->second))};
        } else {
            throw std::invalid_argument("required_cpu field not found");
        }

        if (auto it = params.find("required_bandwidth"); it != params.end()) {
            required_bandwidth_ =
                IOBandwidthMBpS{static_cast<uint32_t>(std::stoi(it->second))};
        } else {
            throw std::invalid_argument("required_bandwidth field not found");
        }
    }

    infra::Workload GetWorkload(TimeStamp time) override
    {
        return {required_ram_, required_cpu_, required_bandwidth_};
    }

 private:
    RAMBytes required_ram_;
    CPUUtilizationPercent required_cpu_;
    IOBandwidthMBpS required_bandwidth_;
};

class GreedyServerScheduler : public IServerScheduler
{
 public:
    explicit GreedyServerScheduler(UUID server_handle)
        : IServerScheduler("Greedy", server_handle)
    {
    }

    void UpdateSchedule() override
    {
        auto server =
            actor_register_->GetActor<infra::Server>(shared_resource_);
        auto server_spec = server->GetSpec();
        const auto& vm_handles = server->GetVMs();

        RAMBytes remaining_ram = server_spec.ram;

        for (const auto& vm_handle : vm_handles) {
            auto vm = actor_register_->GetActor<infra::VM>(vm_handle);

            auto vm_requirements = vm->GetWorkload();

            if (remaining_ram >= vm_requirements.required_ram) {
                remaining_ram -= vm_requirements.required_ram;

                WORLD_LOG_INFO("VM {} is saturated", vm->GetName());

                // TODO: notify vm that it is saturated?
            } else {
                WORLD_LOG_INFO("VM {} is NOT saturated", vm->GetName());
            }
        }
    }
};

}   // namespace sim::custom
