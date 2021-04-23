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
    kVMStopped,
    kVMProvisioned,
    kVMDeleted
};

struct VMStorageEvent : events::Event
{
    VMStorageEventType type{VMStorageEventType::kNone};

    std::string vm_name{};
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
    void InsertVM(const std::shared_ptr<infra::VM>& vm)
    {
        if (vms_.count(vm->GetName())) {
            ACTOR_LOG_ERROR("vm_name {} is not unique", vm->GetName());
            state_ = VMStorageState::kFailure;
        } else {
            vms_[vm->GetName()] = vm;
            ACTOR_LOG_INFO("Created VM with name {}", vm->GetName());
        }
    }

    const auto& PendingVMs() { return pending_vms_; }

    const auto& GetVM(const std::string& vm_name) { return vms_.at(vm_name); }

 private:
    // TODO: this should not be inside VMStorage
    std::unordered_map<std::string, std::shared_ptr<infra::VM>> vms_;

    std::unordered_set<std::string>
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
    void MoveToProvisioning(const std::string& name);
    void MoveToStopped(const std::string& name);
    void MoveToHosted(const std::string& name);
    void DeleteVM(const std::string& name);
};

}   // namespace sim::core
