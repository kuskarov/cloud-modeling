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
    types::UUID vm_uuid;
};

class Server : public IResource
{
 public:
    Server() : IResource("Server") {}

    void HandleEvent(const events::Event* event) override;

    types::EnergyCount SpentPower() override { return 0; }

    // for scheduler
    [[nodiscard]] const auto& VMs() const { return virtual_machines_; }

 private:
    std::unordered_set<types::UUID> virtual_machines_{};

    // event handlers
    void ProvisionVM(const ServerEvent* server_event);
    void UnprovisionVM(const ServerEvent* server_event);
};

struct ServerSpec
{
    types::RAMBytes ram{};
    types::CPUHertz clock_rate{};
    uint32_t cores_count{};
};

}   // namespace sim::infra
