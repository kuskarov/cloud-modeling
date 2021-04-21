#pragma once

#include <unordered_map>
#include <unordered_set>

#include "actor.h"
#include "types.h"
#include "vm.h"

namespace sim::core {

enum class VMStorageEventType
{
    kNone,
    kVMCreated,
    kVMStopped,
    kVMProvisioned,
    kVMDeleted
};

struct VMStorageEvent : events::Event
{
    VMStorageEventType type{VMStorageEventType::kNone};

    types::UUID vm_uuid{};
};

/**
 * A class for VMStorage --- some component of the Cloud, which is responsible
 * for holding VM images (esp. when VM is stopped)
 */
class VMStorage : public events::IActor
{
 public:
    VMStorage() : events::IActor("VM-Storage") {}

    void HandleEvent(const events::Event* event) override;

    // TODO: remove from here
    void InsertVM(types::UUID uuid, const std::shared_ptr<infra::VM>& vm)
    {
        if (vms_.count(uuid)) {
            ACTOR_LOG_ERROR("UUID is not unique");
            state_ = VMStorageState::kFailure;
        } else {
            vms_[uuid] = vm;
        }
    }

    const auto& PendingVMs() { return pending_vms_; }

    const auto& GetVM(types::UUID uuid) { return vms_.at(uuid); }

 private:
    // TODO: this should not be inside VMStorage
    std::unordered_map<types::UUID, std::shared_ptr<infra::VM>> vms_;

    std::unordered_set<types::UUID>
        hosted_vms_{},    // VMs that are currently hosted on some server
        stopped_vms_{},   // VMs that were stopped but not deleted
        pending_vms_{};   // VMs that were created but not provisioned yet

    enum class VMStorageState
    {
        kOk,
        kFailure,
    };

    VMStorageState state_{VMStorageState::kOk};

    // event handlers
    void MoveToProvisioning(types::UUID uuid);
    void MoveToStopped(types::UUID uuid);
    void MoveToHosted(types::UUID uuid);
    void DeleteVM(types::UUID uuid);
};

}   // namespace sim::core
