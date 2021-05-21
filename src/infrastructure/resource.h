#pragma once

#include <exception>
#include <functional>
#include <unordered_set>
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
class IResource : public events::IActor
{
 public:
    explicit IResource(std::string type) : events::IActor(std::move(type)) {}

    void HandleEvent(const events::Event* event) override;

    virtual EnergyCount SpentPower() = 0;

    TimeInterval GetStartupDelay() const;
    void SetStartupDelay(TimeInterval startup_delay);
    TimeInterval GetRebootDelay() const;
    void SetRebootDelay(TimeInterval reboot_delay);
    TimeInterval GetShutdownDelay() const;
    void SetShutdownDelay(TimeInterval shutdown_delay);

    ~IResource() override = default;

 protected:
    /**
     * Set of IResources, which are components of this IResource (e.g.,
     * data-centers are components of the cloud).
     *
     * Is used in turning on/off event handlers
     */
    std::unordered_set<UUID> components_;

    void AddComponent(UUID uuid) { components_.insert(uuid); }

    void SetPowerState(ResourcePowerState new_state);

    ResourcePowerState power_state_{ResourcePowerState::kOff};

    // event handlers
    virtual void StartBoot(const ResourceEvent* resource_event);
    virtual void StartShutdown(const ResourceEvent* resource_event);
    virtual void StartReboot(const ResourceEvent* resource_event);
    virtual void CompleteBoot(const ResourceEvent* resource_event);
    virtual void CompleteShutdown(const ResourceEvent* resource_event);

 private:
    TimeInterval startup_delay_{0}, reboot_delay_{0}, shutdown_delay_{0};
};

}   // namespace sim::infra
