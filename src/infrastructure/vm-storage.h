#pragma once

#include <unordered_map>
#include <unordered_set>

#include "actor.h"
#include "types.h"
#include "vm.h"

namespace sim::infra {

enum class VMStorageEventType
{
    kNone,
    kVMCreated,
    kVMProvisionRequested,
    kVMScheduled,
    kVMHosted,
    kVMStopped,
    kVMDeleted
};

enum class VMStatus
{
    kCreated,     // VM created by no provision request received yet
    kHostedVM,    // VMs that are currently hosted on some server
    kStoppedVM,   // VMs that were stopped but not deleted
    kPending,     // VMs that pending when scheduler decides where to place them
    kProvisioning   // VMs that are set to the server by scheduler bt not hosted
                    // yet
};

struct VMStorageEvent : events::Event
{
    VMStorageEventType type{VMStorageEventType::kNone};
    UUID vm_uuid{};
};

/**
 * A class for VMStorage --- virtual component of the Cloud, which is
 * responsible for holding VM images (esp. when VM is stopped)
 *
 */
class VMStorage : public events::IActor
{
 public:
    VMStorage() : events::IActor("VM-Storage") {}

    void HandleEvent(const events::Event* event) override;

    // For scheduler
    const auto& GetVMs() const { return vms_; }

 private:
    std::unordered_map<UUID, VMStatus> vms_;

    enum class VMStorageState
    {
        kOk,
        kFailure,
    };

    VMStorageState state_{VMStorageState::kOk};

    // event handlers
    void AddVM(const VMStorageEvent* event);
    void MoveToPending(const VMStorageEvent* event);
    void MoveToProvisioning(const VMStorageEvent* event);
    void MoveToStopped(const VMStorageEvent* event);
    void MoveToHosted(const VMStorageEvent* event);
    void DeleteVM(const VMStorageEvent* event);
};

}   // namespace sim::infra
