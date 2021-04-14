#pragma once

#include <exception>
#include <functional>

#include "event.h"
#include "types.h"

namespace sim::resources {
/**
 * Abstract class for Resource (something physical, e.g. Server, DataCenter,
 * Switch, etc.) Each Resource should be able to return SpentPower() in abstract
 * EnergyCount units. Each Resource is an Actor, too.
 */
class Resource : public sim::events::Actor
{
 public:
    Resource() = default;

    virtual sim::types::EnergyCount SpentPower()
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

    void Startup()
    {
        // schedule event "startup_end"
    }

    void Shutdown() {}

    void Reboot() {}

 private:
    sim::types::TimeInterval startup_delay_{}, reboot_delay_{},
        shutdown_delay_{};
};

}   // namespace sim::resources
