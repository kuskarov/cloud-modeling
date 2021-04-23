#include "vm-storage.h"

#include "logger.h"

void
sim::infra::VMStorage::HandleEvent(const sim::events::Event* event)
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

        if (auto it = vms_.find(vm_event->vm_name); it == vms_.end()) {
            ACTOR_LOG_ERROR("Unknown vm_name: {}", vm_event->vm_name);
            state_ = VMStorageState::kFailure;
            return;
        }

        switch (vm_event->type) {
            case VMStorageEventType::kVMCreated: {
                MoveToProvisioning(vm_event->vm_name);
                break;
            }
            case VMStorageEventType::kVMStopped: {
                MoveToStopped(vm_event->vm_name);
                break;
            }
            case VMStorageEventType::kVMProvisioned: {
                MoveToHosted(vm_event->vm_name);
                break;
            }
            case VMStorageEventType::kVMDeleted: {
                DeleteVM(vm_event->vm_name);
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
sim::infra::VMStorage::MoveToProvisioning(const std::string& vm_name)
{
    ACTOR_LOG_INFO("VM {} is being provisioned", vm_name);
    pending_vms_.insert(vm_name);
}

void
sim::infra::VMStorage::MoveToStopped(const std::string& vm_name)
{
    if (auto it = hosted_vms_.find(vm_name); it != hosted_vms_.end()) {
        ACTOR_LOG_INFO("VM {} is stopped and moved to vm-storage", vm_name);
        stopped_vms_.insert(vm_name);
    } else {
        ACTOR_LOG_ERROR("VM {} not found in hosted VM-s list", vm_name);
        state_ = VMStorageState::kFailure;
    }
}

void
sim::infra::VMStorage::MoveToHosted(const std::string& vm_name)
{
    if (auto it = pending_vms_.find(vm_name); it != pending_vms_.end()) {
        ACTOR_LOG_INFO("VM {} is hosted on a server", vm_name);
        hosted_vms_.insert(vm_name);
    } else {
        ACTOR_LOG_ERROR("VM {} not found in pending VM-s list", vm_name);
        state_ = VMStorageState::kFailure;
    }
}

void
sim::infra::VMStorage::DeleteVM(const std::string& vm_name)
{
    if (auto it = hosted_vms_.find(vm_name); it != hosted_vms_.end()) {
        hosted_vms_.erase(it);
        ACTOR_LOG_INFO("VM {} is deleted", vm_name);
    } else if (auto it2 = stopped_vms_.find(vm_name);
               it2 != stopped_vms_.end()) {
        stopped_vms_.erase(it2);
        ACTOR_LOG_INFO("VM {} is deleted", vm_name);
    } else if (auto it3 = pending_vms_.find(vm_name);
               it3 != pending_vms_.end()) {
        pending_vms_.erase(it3);
        ACTOR_LOG_INFO("VM {} is deleted", vm_name);
    } else {
        ACTOR_LOG_ERROR("VM {} not found in VM-s list", vm_name);
        state_ = VMStorageState::kFailure;
    }
}
