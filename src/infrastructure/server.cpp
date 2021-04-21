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
            Resource::HandleEvent(event);
        } else {
            switch (server_event->type) {
                case ServerEventType::kProvisionVM: {
                    ProvisionVM(server_event);
                    break;
                }
                case ServerEventType::kKillVM: {
                    KillVM(server_event);
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
        Resource::HandleEvent(event);
    }
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

void
sim::infra::Server::ProvisionVM(const ServerEvent* server_event)
{
    // add VM to list of hosted VMs and schedule event for VM startup

    if (power_state_ != ResourcePowerState::kRunning) {
        ACTOR_LOG_ERROR(
            "CreateVM event received, but server is not in Running state");
        power_state_ = ResourcePowerState::kFailure;
        return;
    }

    if (!server_event->virtual_machine) {
        ACTOR_LOG_ERROR("CreateVM event without virtual_machine attached");
        return;
    }

    virtual_machines_.push_back(server_event->virtual_machine);

    auto vm_startup_event = new VMEvent();
    vm_startup_event->type = VMEventType::kProvisionCompleted;
    vm_startup_event->addressee = server_event->virtual_machine.get();
    vm_startup_event->happen_time = server_event->happen_time;
    schedule_event(vm_startup_event, true);
}

void
sim::infra::Server::KillVM(const ServerEvent* server_event)
{
}
