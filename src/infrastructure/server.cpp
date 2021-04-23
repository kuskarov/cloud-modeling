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

    if (server_event->vm_name.empty()) {
        ACTOR_LOG_ERROR("ProvisionVM event without virtual_machine attached");
        return;
    }

    auto virtual_machine = vm_storage_->GetVM(server_event->vm_name);
    virtual_machine->SetOwner(this);

    virtual_machines_.insert(server_event->vm_name);
    ACTOR_LOG_INFO("VM {} is hosted here", server_event->vm_name);

    auto vm_provisioned_event = new VMEvent();
    vm_provisioned_event->type = VMEventType::kProvisionCompleted;
    vm_provisioned_event->addressee = virtual_machine.get();
    vm_provisioned_event->happen_time = server_event->happen_time;
    vm_provisioned_event->notificator = server_event->notificator;
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

    if (!virtual_machines_.count(server_event->vm_name)) {
        ACTOR_LOG_ERROR("VM {} is ot hosted on this server",
                        server_event->vm_name);
        return;
    }

    virtual_machines_.erase(server_event->vm_name);
    ACTOR_LOG_INFO("VM {} removed from this server", server_event->vm_name);

    auto event = server_event->notificator;
    event->happen_time = server_event->happen_time;
    schedule_event(event, false);
}

sim::types::RAMBytes
sim::infra::Server::GetRam() const
{
    return ram_;
}

void
sim::infra::Server::SetRam(types::RAMBytes ram)
{
    ram_ = ram;
}

sim::types::CPUHertz
sim::infra::Server::GetClockRate() const
{
    return clock_rate_;
}

void
sim::infra::Server::SetClockRate(types::CPUHertz clock_rate)
{
    clock_rate_ = clock_rate;
}

uint32_t
sim::infra::Server::GetCoreCount() const
{
    return cores_count_;
}

void
sim::infra::Server::SetCoresCount(uint32_t cores_count)
{
    cores_count_ = cores_count;
}

sim::types::Currency
sim::infra::Server::GetCost() const
{
    return cost_;
}

void
sim::infra::Server::SetCost(types::Currency cost)
{
    cost_ = cost;
}
