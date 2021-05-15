#include "server.h"

#include <typeinfo>

#include "event.h"
#include "logger.h"
#include "vm.h"

void
sim::infra::Server::HandleEvent(const events::Event* event)
{
    try {
        auto server_event = dynamic_cast<const ServerEvent*>(event);

        if (!server_event) {
            IResource::HandleEvent(event);
        } else {
            switch (server_event->type) {
                case ServerEventType::kProvisionVM: {
                    ProvisionVM(server_event);
                    break;
                }
                case ServerEventType::kUnprovisionVM: {
                    UnprovisionVM(server_event);
                    break;
                }
                default: {
                    ACTOR_LOG_ERROR("Received server event with invalid type");
                    break;
                }
            }
        }
    } catch (const std::bad_cast& bc) {
        // may be an event of underlying class
        IResource::HandleEvent(event);
    }
}

void
sim::infra::Server::ProvisionVM(const ServerEvent* server_event)
{
    // add VM to list of hosted VMs and schedule event for VM startup

    if (power_state_ != ResourcePowerState::kRunning) {
        ACTOR_LOG_ERROR(
            "ProvisionVM event received, but server is not in Running state");
        power_state_ = ResourcePowerState::kFailure;
        return;
    }

    if (!server_event->vm_uuid) {
        ACTOR_LOG_ERROR("ProvisionVM event without virtual_machine attached");
        return;
    }

    virtual_machines_.insert(server_event->vm_uuid);
    ACTOR_LOG_INFO("VM {} is hosted here", server_event->vm_uuid);

    auto vm_provisioned_event = events::MakeInheritedEvent<VMEvent>(
        server_event->vm_uuid, server_event, types::TimeInterval{0});
    vm_provisioned_event->type = VMEventType::kProvisionCompleted;
    vm_provisioned_event->server_uuid = UUID();

    schedule_event(vm_provisioned_event, false);
}

void
sim::infra::Server::UnprovisionVM(const ServerEvent* server_event)
{
    // remove VM from list of hosted VMs

    if (power_state_ != ResourcePowerState::kRunning) {
        ACTOR_LOG_ERROR(
            "UnprovisionVM event received, but server is not in Running state");
        power_state_ = ResourcePowerState::kFailure;
        return;
    }

    if (!virtual_machines_.count(server_event->vm_uuid)) {
        ACTOR_LOG_ERROR("VM {} is ot hosted on this server",
                        server_event->vm_uuid);
        return;
    }

    virtual_machines_.erase(server_event->vm_uuid);
    ACTOR_LOG_INFO("VM {} removed from this server", server_event->vm_uuid);

    auto event = server_event->notificator;
    event->happen_time = server_event->happen_time;
    schedule_event(event, false);
}
