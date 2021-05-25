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
    uint32_t cores_count{};
    IOBandwidthMBpS io_bandwidth{};
};

class Server : public IResource
{
 public:
    Server() : IResource("Server") {}

    void HandleEvent(const events::Event* event) override;

    // for scheduler
    const auto& GetVMs() const { return virtual_machines_; }

    void SetSpec(ServerSpec spec) { spec_ = spec; }

    auto GetSpec() const { return spec_; }

    void SetWorkload(Workload workload) const { server_workload_ = workload; }

 private:
    ServerSpec spec_{};

    mutable Workload server_workload_{};

    // consumers of Server as a resource
    std::unordered_set<UUID> virtual_machines_{};

    // event handlers
    void ProvisionVM(const ServerEvent* server_event);
    void UnprovisionVM(const ServerEvent* server_event);
};

}   // namespace sim::infra
