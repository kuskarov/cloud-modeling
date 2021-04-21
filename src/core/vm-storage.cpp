#include "vm-storage.h"

#include "logger.h"

void
sim::core::VMStorage::HandleEvent(const sim::events::Event* event)
{
    if (state_ != VMStorageState::kOk) {
        ACTOR_LOG_ERROR("In failed state");
        return;
    }

    try {
        auto vm_event = dynamic_cast<const VMStorageEvent*>(event);
        if (!vm_event) {
            ACTOR_LOG_ERROR("Received invalid event");
            state_ = VMStorageState::kFailure;
            return;
        }

        if (auto it = vms_.find(vm_event->vm_uuid); it == vms_.end()) {
            ACTOR_LOG_ERROR("Unknown UUID: {}", vm_event->vm_uuid);
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
                ACTOR_LOG_ERROR("Received unknown event");
                state_ = VMStorageState::kFailure;
            }
        }

    } catch (std::bad_cast& bc) {
        ACTOR_LOG_ERROR("Received invalid event");
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
        ACTOR_LOG_ERROR("VM {} not found in hosted VM-s list", uuid);
        state_ = VMStorageState::kFailure;
    }
}

void
sim::core::VMStorage::MoveToHosted(types::UUID uuid)
{
    if (auto it = pending_vms_.find(uuid); it != pending_vms_.end()) {
        hosted_vms_.insert(uuid);
    } else {
        ACTOR_LOG_ERROR("VM {} not found in pending VM-s list", uuid);
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
        ACTOR_LOG_ERROR("VM {} not found in VM-s list", uuid);
        state_ = VMStorageState::kFailure;
    }
}
