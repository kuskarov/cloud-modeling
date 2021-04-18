#pragma once

#include <exception>
#include <functional>
#include <utility>

#include "actor.h"
#include "event.h"
#include "types.h"

namespace sim::resources {

enum class ResourceEventType
{
    kNone,
    kBoot,               // called when the server is Off
    kReboot,             // called when the server is Running
    kBootFinished,       // scheduled by kStartup and kReboot handlers
    kShutdown,           // called when the server is Running
    kShutdownFinished,   // scheduled by kShutdown
};

enum class ResourcePowerState
{
    kOff = 0,
    kTurningOn,
    kTurningOff,
    kRunning,
    kFailure,
};

struct ResourceEvent : events::Event
{
    ResourceEventType resource_event_type{ResourceEventType::kNone};
};

/**
 * Abstract class for Resource (something physical, e.g. Server, DataCenter,
 * Switch, etc.) Each Resource:
 *
 * 1) should be able to return SpentPower() in abstract EnergyCount units
 * 2) is an Actor
 * 3) has PowerState (Off, TurningOn/Off, Running, Failure)
 * 4) inherits standard life cycle and ResourceEvent handling
 * 5) has startup, reboot and shutdown delays
 *
 */
class Resource : public events::Actor
{
 public:
    explicit Resource(std::string type) : events::Actor(std::move(type)) {}

    void HandleEvent(const std::shared_ptr<events::Event>& event) override;

    virtual types::EnergyCount SpentPower()
    {
        throw std::logic_error(
            "abstract class method invocation! not implemented");
    };

    [[nodiscard]] types::TimeInterval GetStartupDelay() const
    {
        return startup_delay_;
    }

    void SetStartupDelay(types::TimeInterval startup_delay)
    {
        startup_delay_ = startup_delay;
    }

    [[nodiscard]] types::TimeInterval GetRebootDelay() const
    {
        return reboot_delay_;
    }

    void SetRebootDelay(types::TimeInterval reboot_delay)
    {
        reboot_delay_ = reboot_delay;
    }

    [[nodiscard]] types::TimeInterval GetShutdownDelay() const
    {
        return shutdown_delay_;
    }

    void SetShutdownDelay(types::TimeInterval shutdown_delay)
    {
        shutdown_delay_ = shutdown_delay;
    }

    ~Resource() override = default;

 protected:
    ResourcePowerState power_state_{ResourcePowerState::kOff};

    bool PowerStateIs(ResourcePowerState expected,
                      const std::string& caller_info);

    static inline std::string PowerStateToString(ResourcePowerState state)
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

    types::TimeInterval startup_delay_{1}, reboot_delay_{1}, shutdown_delay_{1};
};

}   // namespace sim::resources
