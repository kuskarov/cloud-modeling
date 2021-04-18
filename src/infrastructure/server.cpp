#include "server.h"

#include <loguru.hpp>
#include <typeinfo>

#include "event.h"

void
sim::resources::Server::HandleEvent(const std::shared_ptr<events::Event>& event)
{
    try {
        auto server_event = dynamic_cast<const ServerEvent*>(event.get());

        if (!server_event) {
            Resource::HandleEvent(event);
        } else {
            switch (server_event->type) {
                case ServerEventType::kProvisionVM: {
                    // add VM to list of hosted VMs and schedule event for VM
                    // startup

                    if (power_state_ != ResourcePowerState::kRunning) {
                        LOG_F(
                            ERROR,
                            "ProvisionVM event received, but server is not in "
                            "Running state");
                        power_state_ = ResourcePowerState::kFailure;
                        break;
                    }

                    if (!server_event->virtual_machine) {
                        LOG_F(ERROR,
                              "ProvisionVM event without virtual_machine "
                              "attached");
                        break;
                    }

                    virtual_machines_.push_back(server_event->virtual_machine);

                    // TODO: schedule event

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
sim::resources::Server::GetRam() const
{
    return ram_;
}

void
sim::resources::Server::SetRam(sim::types::RAMBytes ram)
{
    ram_ = ram;
}

sim::types::CPUHertz
sim::resources::Server::GetClockRate() const
{
    return clock_rate_;
}

void
sim::resources::Server::SetClockRate(sim::types::CPUHertz clock_rate)
{
    clock_rate_ = clock_rate;
}

uint32_t
sim::resources::Server::GetCoreCount() const
{
    return cores_count_;
}

void
sim::resources::Server::SetCoresCount(uint32_t cores_count)
{
    cores_count_ = cores_count;
}

sim::types::Currency
sim::resources::Server::GetCost() const
{
    return cost_;
}

void
sim::resources::Server::SetCost(sim::types::Currency cost)
{
    cost_ = cost;
}
