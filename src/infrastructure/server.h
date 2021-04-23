#pragma once

#include <unordered_set>

#include "actor.h"
#include "event.h"
#include "resource.h"
#include "types.h"
#include "vm-storage.h"
#include "vm.h"

namespace sim::infra {

enum class ServerEventType
{
    kNone,
    kProvisionVM,
    kUnprovisionVM
};

struct ServerEvent : events::Event
{
    ServerEventType type{ServerEventType::kNone};
    std::string vm_name;
};

class Server : public IResource
{
 public:
    Server() : IResource("Server") {}

    void HandleEvent(const events::Event* event) override;

    types::EnergyCount SpentPower() override { return 0; }

    // for scheduler
    [[nodiscard]] const auto& VMs() const { return virtual_machines_; }

    [[nodiscard]] types::RAMBytes GetRam() const;
    void SetRam(types::RAMBytes ram);
    [[nodiscard]] types::CPUHertz GetClockRate() const;
    void SetClockRate(types::CPUHertz clock_rate);
    [[nodiscard]] uint32_t GetCoreCount() const;
    void SetCoresCount(uint32_t cores_count);
    [[nodiscard]] types::Currency GetCost() const;
    void SetCost(types::Currency cost);

    void SetVMStorage(VMStorage* vm_storage) { vm_storage_ = vm_storage; }

 private:
    std::unordered_set<std::string> virtual_machines_{};

    VMStorage* vm_storage_{};

    types::RAMBytes ram_{};
    types::CPUHertz clock_rate_{};
    uint32_t cores_count_{};
    types::Currency cost_{};

    // event handlers
    void ProvisionVM(const ServerEvent* server_event);
    void UnprovisionVM(const ServerEvent* server_event);
};

}   // namespace sim::infra
