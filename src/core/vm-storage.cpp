#include "vm-storage.h"

void
sim::core::VMStorage::HandleEvent(const std::shared_ptr<events::Event>& event)
{
    if (state_ != VMStorageState::kOk) {
        LOG_F(ERROR, "VM-Storage failed");
        return;
    }

    try {
        auto vm_event = dynamic_cast<const VMStorageEvent*>(event.get());
        if (!vm_event) {
            LOG_F(ERROR, "VM-Storage received invalid event");
            state_ = VMStorageState::kFailure;
            return;
        }

        if (auto it = vms_.find(vm_event->vm_uuid); it == vms_.end()) {
            LOG_F(ERROR, "Unknown UUID: %lu", vm_event->vm_uuid);
            state_ = VMStorageState::kFailure;
            return;
        }

        switch (vm_event->type) {
            case VMStorageEventType::kVMCreated: {
                MoveToProvisioning(vm_event->vm_uuid);
                break;
            }
            case VMStorageEventType::kVMStopped: {
                MoveToStopped(vm_event->vm_uuid);
                break;
            }
            case VMStorageEventType::kVMProvisioned: {
                MoveToHosted(vm_event->vm_uuid);
                break;
            }
            case VMStorageEventType::kVMDeleted: {
                DeleteVM(vm_event->vm_uuid);
                break;
            }
            default: {
                LOG_F(ERROR, "VM-Storage received unknown event");
                state_ = VMStorageState::kFailure;
            }
        }

    } catch (std::bad_cast& bc) {
        LOG_F(ERROR, "VM-Storage received invalid event");
        state_ = VMStorageState::kFailure;
    }
}

void
sim::core::VMStorage::MoveToProvisioning(types::UUID uuid)
{
    pending_vms_.insert(uuid);
}

void
sim::core::VMStorage::MoveToStopped(types::UUID uuid)
{
    if (auto it = hosted_vms_.find(uuid); it != hosted_vms_.end()) {
        stopped_vms_.insert(uuid);
    } else {
        LOG_F(ERROR, "VM-Storage: VM %lu not found in hosted VM-s list", uuid);
        state_ = VMStorageState::kFailure;
    }
}

void
sim::core::VMStorage::MoveToHosted(types::UUID uuid)
{
    if (auto it = pending_vms_.find(uuid); it != pending_vms_.end()) {
        hosted_vms_.insert(uuid);
    } else {
        LOG_F(ERROR, "VM-Storage: VM %lu not found in pending VM-s list", uuid);
        state_ = VMStorageState::kFailure;
    }
}

void
sim::core::VMStorage::DeleteVM(types::UUID uuid)
{
    if (auto it = hosted_vms_.find(uuid); it != hosted_vms_.end()) {
        hosted_vms_.erase(it);
    } else if (auto it2 = stopped_vms_.find(uuid); it2 != stopped_vms_.end()) {
        stopped_vms_.erase(it2);
    } else if (auto it3 = pending_vms_.find(uuid); it3 != pending_vms_.end()) {
        pending_vms_.erase(it3);
    } else {
        LOG_F(ERROR, "VM-Storage: VM %lu not found in VM-s list", uuid);
        state_ = VMStorageState::kFailure;
    }
}
