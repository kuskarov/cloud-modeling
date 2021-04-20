#include "server.h"

#include <loguru.hpp>
#include <typeinfo>

#include "event.h"
#include "vm.h"

void
sim::infra::Server::HandleEvent(const std::shared_ptr<events::Event>& event)
{
    try {
        auto server_event = dynamic_cast<const ServerEvent*>(event.get());

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
                    LOG_F(ERROR,
                          "Server %s received server event with invalid type",
                          name_.c_str());
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
sim::infra::Server::SetRam(sim::types::RAMBytes ram)
{
    ram_ = ram;
}

sim::types::CPUHertz
sim::infra::Server::GetClockRate() const
{
    return clock_rate_;
}

void
sim::infra::Server::SetClockRate(sim::types::CPUHertz clock_rate)
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
sim::infra::Server::SetCost(sim::types::Currency cost)
{
    cost_ = cost;
}

void
sim::infra::Server::ProvisionVM(const sim::infra::ServerEvent* server_event)
{
    // add VM to list of hosted VMs and schedule event for VM startup

    if (power_state_ != ResourcePowerState::kRunning) {
        LOG_F(ERROR,
              "CreateVM event received, but server is not in "
              "Running state");
        power_state_ = ResourcePowerState::kFailure;
        return;
    }

    if (!server_event->virtual_machine) {
        LOG_F(ERROR,
              "CreateVM event without virtual_machine "
              "attached");
        return;
    }

    virtual_machines_.push_back(server_event->virtual_machine);

    auto vm_startup_event = std::make_shared<VMEvent>();
    vm_startup_event->type = VMEventType::kProvisionCompleted;
    vm_startup_event->addressee = server_event->virtual_machine.get();
    vm_startup_event->happen_ts = server_event->happen_ts;
    schedule_callback_(server_event->happen_ts, vm_startup_event, true);
}

void
sim::infra::Server::KillVM(const sim::infra::ServerEvent* server_event)
{
}
