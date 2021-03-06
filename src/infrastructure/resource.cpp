#include "resource.h"

#include "logger.h"

std::string_view
sim::infra::IResource::PowerStateToString(PowerState state)
{
    switch (state) {
        case sim::infra::IResource::PowerState::kOff:
            return "OFF";
        case sim::infra::IResource::PowerState::kRunning:
            return "RUNNING";
        case sim::infra::IResource::PowerState::kTurningOn:
            return "TURNING_ON";
        case sim::infra::IResource::PowerState::kTurningOff:
            return "TURNING_OFF";
        case sim::infra::IResource::PowerState::kFailure:
            return "FAILURE";
        default:
            abort();
    }
}

#define CHECK_POWER_STATE(expected, extra_text)                          \
    if (power_state_ != (expected)) {                                    \
        ACTOR_LOG_ERROR("{}: current power state is {} but expected {}", \
                        extra_text, PowerStateToString(power_state_),    \
                        PowerStateToString(expected));                   \
        SetPowerState(PowerState::kFailure);                             \
        return;                                                          \
    }

void
sim::infra::IResource::HandleEvent(const events::Event* event)
{
    auto resource_event = dynamic_cast<const ResourceEvent*>(event);

    if (!resource_event) {
        ACTOR_LOG_ERROR("Received invalid event");
        return;
    }

    switch (resource_event->type) {
        case ResourceEventType::kBoot:
            StartBoot(resource_event);
            break;
        case ResourceEventType::kReboot:
            StartReboot(resource_event);
            break;
        case ResourceEventType::kBootFinished:
            CompleteBoot(resource_event);
            break;
        case ResourceEventType::kShutdown:
            StartShutdown(resource_event);
            break;
        case ResourceEventType::kShutdownFinished:
            CompleteShutdown(resource_event);
            break;
        default: {
            ACTOR_LOG_ERROR("Received event with invalid type");
            break;
        }
    }
}

void
sim::infra::IResource::StartBoot(const ResourceEvent* resource_event)
{
    if (power_state_ == PowerState::kFailure) {
        ACTOR_LOG_ERROR("I am failed");
    } else if (power_state_ == PowerState::kOff) {
        SetPowerState(PowerState::kTurningOn);

        for (auto component : components_) {
            auto boot_component_event = events::MakeEvent<ResourceEvent>(
                component, resource_event->happen_time + startup_delay_,
                nullptr);
            boot_component_event->type = ResourceEventType::kBoot;

            schedule_event(boot_component_event, false);
        }

        auto boot_finished_event = events::MakeEvent<ResourceEvent>(
            GetUUID(), resource_event->happen_time + startup_delay_, nullptr);
        boot_finished_event->type = ResourceEventType::kBootFinished;

        schedule_event(boot_finished_event, false);
    } else {
        ACTOR_LOG_INFO("Already ON");
    }
}

void
sim::infra::IResource::StartShutdown(const ResourceEvent* resource_event)
{
    // switch state to kTurningOff and schedule kShutdownFinished event
    if (power_state_ == PowerState::kOff) {
        ACTOR_LOG_INFO("Already OFF");
    } else {
        SetPowerState(PowerState::kTurningOff);

        for (auto component : components_) {
            auto shutdown_component_event = events::MakeEvent<ResourceEvent>(
                component, resource_event->happen_time + shutdown_delay_,
                nullptr);
            shutdown_component_event->type = ResourceEventType::kShutdown;

            schedule_event(shutdown_component_event, false);
        }

        auto shutdown_finished_event = events::MakeEvent<ResourceEvent>(
            GetUUID(), resource_event->happen_time + shutdown_delay_, nullptr);
        shutdown_finished_event->type = ResourceEventType::kShutdownFinished;

        schedule_event(shutdown_finished_event, false);
    }
}

void
sim::infra::IResource::StartReboot(const ResourceEvent* resource_event)
{
}

void
sim::infra::IResource::CompleteBoot(const ResourceEvent* resource_event)
{
    // switch state to kRunning

    CHECK_POWER_STATE(PowerState::kTurningOn, "BootFinished Event Handler");

    SetPowerState(PowerState::kRunning);
}

void
sim::infra::IResource::CompleteShutdown(const ResourceEvent* resource_event)
{
    // switch state to kOff

    CHECK_POWER_STATE(PowerState::kTurningOff,
                      "ShutdownFinished Event Handler");

    SetPowerState(PowerState::kOff);
}

sim::TimeInterval
sim::infra::IResource::GetStartupDelay() const
{
    return startup_delay_;
}

void
sim::infra::IResource::SetStartupDelay(TimeInterval startup_delay)
{
    startup_delay_ = startup_delay;
}

sim::TimeInterval
sim::infra::IResource::GetRebootDelay() const
{
    return reboot_delay_;
}

void
sim::infra::IResource::SetRebootDelay(TimeInterval reboot_delay)
{
    reboot_delay_ = reboot_delay;
}

sim::TimeInterval
sim::infra::IResource::GetShutdownDelay() const
{
    return shutdown_delay_;
}

void
sim::infra::IResource::SetShutdownDelay(TimeInterval shutdown_delay)
{
    shutdown_delay_ = shutdown_delay;
}

sim::EnergyCount
sim::infra::IResource::GetEnergyPerTickConst() const
{
    return energy_per_tick_const_;
}

void
sim::infra::IResource::SetEnergyPerTickConst(
    sim::EnergyCount energy_per_tick_const)
{
    energy_per_tick_const_ = energy_per_tick_const;
}

void
sim::infra::IResource::SetPowerState(PowerState new_state)
{
    power_state_ = new_state;
    ACTOR_LOG_INFO("State changed to {}", PowerStateToString(new_state));
}
