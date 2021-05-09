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
        auto vms_event = dynamic_cast<const VMStorageEvent*>(event);
        if (!vms_event) {
            ACTOR_LOG_ERROR("Received invalid event");
            state_ = VMStorageState::kFailure;
            return;
        }

        switch (vms_event->type) {
            case VMStorageEventType::kVMCreated: {
                AddVM(vms_event);
                break;
            }
            case VMStorageEventType::kVMProvisionRequested: {
                MoveToProvisioning(vms_event);
                break;
            }
            case VMStorageEventType::kVMStopped: {
                MoveToStopped(vms_event);
                break;
            }
            case VMStorageEventType::kVMProvisioned: {
                MoveToHosted(vms_event);
                break;
            }
            case VMStorageEventType::kVMDeleted: {
                DeleteVM(vms_event);
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
sim::infra::VMStorage::AddVM(const sim::infra::VMStorageEvent* event)
{
    if (auto it = vms_.find(event->vm_uuid); it == vms_.end()) {
        vms_[event->vm_uuid] = VMStatus::kPendingProvisionVM;
        ACTOR_LOG_INFO("VM {} is added", event->vm_uuid);
    } else {
        ACTOR_LOG_ERROR("VM {} not found in VM-s list", event->vm_uuid);
        state_ = VMStorageState::kFailure;
    }
}

// TODO: boilerplate
void
sim::infra::VMStorage::MoveToProvisioning(const VMStorageEvent* event)
{
    if (auto it = vms_.find(event->vm_uuid); it != vms_.end()) {
        if (it->second != VMStatus::kPendingProvisionVM) {
            ACTOR_LOG_INFO("VM {} is being provisioned", event->vm_uuid);
            it->second = VMStatus::kPendingProvisionVM;
        } else {
            ACTOR_LOG_ERROR("VM {} is already being provisioned",
                            event->vm_uuid);
        }
    } else {
        ACTOR_LOG_ERROR("VM {} not found in VM-s list", event->vm_uuid);
        state_ = VMStorageState::kFailure;
    }
}

void
sim::infra::VMStorage::MoveToStopped(const VMStorageEvent* event)
{
    if (auto it = vms_.find(event->vm_uuid); it != vms_.end()) {
        if (it->second != VMStatus::kStoppedVM) {
            ACTOR_LOG_INFO("VM {} is stopped", event->vm_uuid);
            it->second = VMStatus::kStoppedVM;
        } else {
            ACTOR_LOG_ERROR("VM {} is already stopped", event->vm_uuid);
        }
    } else {
        ACTOR_LOG_ERROR("VM {} not found in VM-s list", event->vm_uuid);
        state_ = VMStorageState::kFailure;
    }
}

void
sim::infra::VMStorage::MoveToHosted(const VMStorageEvent* event)
{
    if (auto it = vms_.find(event->vm_uuid); it != vms_.end()) {
        if (it->second != VMStatus::kHostedVM) {
            ACTOR_LOG_INFO("VM {} is hosted on a server", event->vm_uuid);
            it->second = VMStatus::kHostedVM;
        } else {
            ACTOR_LOG_ERROR("VM {} is already hosted", event->vm_uuid);
        }
    } else {
        ACTOR_LOG_ERROR("VM {} not found in VM-s list", event->vm_uuid);
        state_ = VMStorageState::kFailure;
    }
}

void
sim::infra::VMStorage::DeleteVM(const VMStorageEvent* event)
{
    if (auto it = vms_.find(event->vm_uuid); it != vms_.end()) {
        vms_.erase(it);
        ACTOR_LOG_INFO("VM {} is deleted", event->vm_uuid);
    } else {
        ACTOR_LOG_ERROR("VM {} not found in VM-s list", event->vm_uuid);
        state_ = VMStorageState::kFailure;
    }
}
