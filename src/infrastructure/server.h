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
    UUID vm_uuid;
};

struct ServerSpec
{
    RAMBytes ram{};
    CPUHertz clock_rate{};
    uint32_t cores_count{};
};

class Server : public IResource
{
 public:
    Server() : IResource("Server") {}

    void HandleEvent(const events::Event* event) override;

    EnergyCount SpentPower() override { return EnergyCount{0}; }

    // for scheduler
    const auto& VMs() const { return virtual_machines_; }

    void SetSpec(ServerSpec spec) { spec_ = spec; }

    auto GetSpec() const { return spec_; }

 private:
    ServerSpec spec_{};

    // consumers of Server as a resource
    std::unordered_set<UUID> virtual_machines_{};

    // event handlers
    void ProvisionVM(const ServerEvent* server_event);
    void UnprovisionVM(const ServerEvent* server_event);
};

}   // namespace sim::infra
