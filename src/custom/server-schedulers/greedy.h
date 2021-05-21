#pragma once

#include "resource-scheduler.h"

namespace sim::custom {

using namespace sim::core;

struct RAMVMWorkload : infra::IVMWorkload
{
    RAMBytes required_ram{};
};

class RAMConstVMWorkloadModel : public infra::IWorkloadModel
{
 public:
    void Setup(
        const std::unordered_map<std::string, std::string>& params) override
    {
        if (auto it = params.find("required_ram"); it != params.end()) {
            required_ram_ =
                RAMBytes{static_cast<uint32_t>(std::stoi(it->second))};
        }
    }

    std::shared_ptr<infra::IVMWorkload> GetWorkload(TimeStamp time) override
    {
        auto wl = std::make_shared<RAMVMWorkload>();
        wl->required_ram = required_ram_;

        return wl;
    }

 private:
    RAMBytes required_ram_;
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
        const auto& vm_handles = server->VMs();

        RAMBytes remaining_ram = server_spec.ram;

        for (const auto& vm_handle : vm_handles) {
            auto vm = actor_register_->GetActor<infra::VM>(vm_handle);

            auto vm_requirements =
                std::static_pointer_cast<RAMVMWorkload>(vm->GetWorkload());

            if (!vm_requirements) {
                WORLD_LOG_ERROR("VM Workload type mismatch");
                return;
            }

            if (remaining_ram >= vm_requirements->required_ram) {
                remaining_ram -= vm_requirements->required_ram;

                WORLD_LOG_INFO("VM {} is saturated", vm->GetName());

                // TODO: notify vm that it is saturated?
            } else {
                WORLD_LOG_INFO("VM {} is NOT saturated", vm->GetName());
            }
        }
    }
};

}   // namespace sim::custom
