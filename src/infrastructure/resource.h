#pragma once

#include <exception>
#include <functional>
#include <utility>

#include "actor.h"
#include "event.h"
#include "types.h"

namespace sim::infra {

enum class ResourceEventType
{
    kNone,
    kBoot,           // external command, Resource should be in Off state
    kReboot,         // external command, Resource should be in Running state
    kBootFinished,   // scheduled in Boot and Reboot handlers
    kShutdown,       // external command, Resource should be in Running state
    kShutdownFinished,   // scheduled by kShutdown
};

struct ResourceEvent : events::Event
{
    ResourceEventType type{ResourceEventType::kNone};
};

enum class ResourcePowerState
{
    kOff,
    kTurningOn,
    kTurningOff,
    kRunning,
    kFailure,
};

static inline const char* PowerStateToString(ResourcePowerState state);

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

    void HandleEvent(const events::Event* event) override;

    // TODO: set =0 and implement in derived
    virtual types::EnergyCount SpentPower() { return 0; }

    [[nodiscard]] types::TimeInterval GetStartupDelay() const;
    void SetStartupDelay(types::TimeInterval startup_delay);
    [[nodiscard]] types::TimeInterval GetRebootDelay() const;
    void SetRebootDelay(types::TimeInterval reboot_delay);
    [[nodiscard]] types::TimeInterval GetShutdownDelay() const;
    void SetShutdownDelay(types::TimeInterval shutdown_delay);

    ~Resource() override = default;

 protected:
    inline bool PowerStateIs(ResourcePowerState expected,
                             const std::string& caller_info);

    ResourcePowerState power_state_{ResourcePowerState::kOff};

    /// TODO: remove magic numbers
    types::TimeInterval startup_delay_{3}, reboot_delay_{4}, shutdown_delay_{1};

    // event handlers
    virtual void StartBoot(const ResourceEvent* resource_event);
    virtual void StartShutdown(const ResourceEvent* resource_event);
    virtual void StartReboot(const ResourceEvent* resource_event);
    virtual void CompleteBoot(const ResourceEvent* resource_event);
    virtual void CompleteShutdown(const ResourceEvent* resource_event);
};

}   // namespace sim::infra
