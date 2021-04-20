#include "resource.h"

#include <loguru.hpp>

void
sim::infra::Resource::HandleEvent(const sim::events::Event* event)
{
    try {
        auto resource_event = dynamic_cast<const ResourceEvent*>(event);

        if (!resource_event) {
            LOG_F(ERROR, "Resource Event is null in %s %s", type_.c_str(),
                  name_.c_str());
            return;
        }

        switch (resource_event->resource_event_type) {
            case ResourceEventType::kBoot: {
                StartBoot(resource_event);
                break;
            }
            case ResourceEventType::kReboot: {
                StartReboot(resource_event);
                break;
            }
            case ResourceEventType::kBootFinished: {
                CompleteBoot(resource_event);
                break;
            }
            case ResourceEventType::kShutdown: {
                StartShutdown(resource_event);
                break;
            }
            case ResourceEventType::kShutdownFinished: {
                CompleteShutdown(resource_event);
                break;
            }
            default: {
                LOG_F(ERROR,
                      "resource %s received resource event with invalid type",
                      name_.c_str());
                break;
            }
        }

    } catch (const std::bad_cast& bc) {
        LOG_F(ERROR, "Resource %s unknown event!", name_.c_str());
    }
}

sim::types::EnergyCount
sim::infra::Resource::SpentPower()
{
    throw std::logic_error("abstract class method invocation! not implemented");
};

sim::types::TimeInterval
sim::infra::Resource::GetStartupDelay() const
{
    return startup_delay_;
}

void
sim::infra::Resource::SetStartupDelay(types::TimeInterval startup_delay)
{
    startup_delay_ = startup_delay;
}

sim::types::TimeInterval
sim::infra::Resource::GetRebootDelay() const
{
    return reboot_delay_;
}

void
sim::infra::Resource::SetRebootDelay(types::TimeInterval reboot_delay)
{
    reboot_delay_ = reboot_delay;
}

sim::types::TimeInterval
sim::infra::Resource::GetShutdownDelay() const
{
    return shutdown_delay_;
}

void
sim::infra::Resource::SetShutdownDelay(types::TimeInterval shutdown_delay)
{
    shutdown_delay_ = shutdown_delay;
}

bool
sim::infra::Resource::PowerStateIs(sim::infra::ResourcePowerState expected,
                                   const std::string& caller_info)
{
    if (power_state_ != expected) {
        LOG_F(ERROR, "%s: given state %s, expected %s", caller_info.c_str(),
              PowerStateToString(power_state_), PowerStateToString(expected));

        power_state_ = ResourcePowerState::kFailure;

        return false;
    }
    return true;
}

const char*
sim::infra::PowerStateToString(ResourcePowerState state)
{
    switch (state) {
        case ResourcePowerState::kOff:
            return "OFF";
        case ResourcePowerState::kRunning:
            return "RUNNING";
        case ResourcePowerState::kTurningOn:
            return "TURNING_ON";
        case ResourcePowerState::kTurningOff:
            return "TURNING_OFF";
        case ResourcePowerState::kFailure:
            return "FAILURE";
        default:
            return "UNREACHABLE";
    }
}

void
sim::infra::Resource::StartBoot(const ResourceEvent* resource_event)
{
    // switch state to kTurningOn and schedule kBootFinished event
    LOG_F(INFO, "Resource::StartBoot");

    if (!PowerStateIs(ResourcePowerState::kOff, "Boot Event Handler")) {
        return;
    }

    power_state_ = ResourcePowerState::kTurningOn;
    LOG_F(INFO, "State changed to %s", PowerStateToString(power_state_));

    auto boot_finished_event = new ResourceEvent();
    boot_finished_event->addressee = this;
    boot_finished_event->resource_event_type = ResourceEventType::kBootFinished;
    boot_finished_event->happen_time =
        resource_event->happen_time + startup_delay_;

    schedule_event(boot_finished_event, false);
}

void
sim::infra::Resource::StartShutdown(const ResourceEvent* resource_event)
{
    // switch state to kTurningOff and schedule kShutdownFinished event
    if (!PowerStateIs(ResourcePowerState::kRunning, "Shutdown Event Handler")) {
        return;
    }

    power_state_ = ResourcePowerState::kTurningOff;
    LOG_F(INFO, "State changed to %s", PowerStateToString(power_state_));

    auto shutdown_finished_event = new ResourceEvent();
    shutdown_finished_event->addressee = this;
    shutdown_finished_event->resource_event_type =
        ResourceEventType::kShutdownFinished;
    shutdown_finished_event->happen_time =
        resource_event->happen_time + shutdown_delay_;

    schedule_event(shutdown_finished_event, false);
}

void
sim::infra::Resource::StartReboot(const ResourceEvent* resource_event)
{
}

void
sim::infra::Resource::CompleteBoot(const ResourceEvent* resource_event)
{
    // switch state to kRunning

    if (!PowerStateIs(ResourcePowerState::kTurningOn,
                      "BootFinished Event Handler")) {
        return;
    }

    power_state_ = ResourcePowerState::kRunning;
    LOG_F(INFO, "State changed to %s", PowerStateToString(power_state_));
}

void
sim::infra::Resource::CompleteShutdown(const ResourceEvent* resource_event)
{
    // switch state to kOff

    if (!PowerStateIs(ResourcePowerState::kTurningOff,
                      "ShutdownFinished Event Handler")) {
        return;
    }

    power_state_ = ResourcePowerState::kOff;
    LOG_F(INFO, "State changed to %s", PowerStateToString(power_state_));
}
