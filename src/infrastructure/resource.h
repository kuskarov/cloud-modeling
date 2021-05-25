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

typedef std::function<EnergyCount(UUID)> EnergyMeterFunction;

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
    void HandleEvent(const events::Event* event) override;

    virtual EnergyCount SpentPower() {
        EnergyCount total{energy_per_tick_const_};

        for (UUID component_handle : components_) {
            total += energy_meter_function_(component_handle);
        }

        return total;
    };

    TimeInterval GetStartupDelay() const;
    void SetStartupDelay(TimeInterval startup_delay);
    TimeInterval GetRebootDelay() const;
    void SetRebootDelay(TimeInterval reboot_delay);
    TimeInterval GetShutdownDelay() const;
    void SetShutdownDelay(TimeInterval shutdown_delay);
    EnergyCount GetEnergyPerTickConst() const;
    void SetEnergyPerTickConst(EnergyCount energy_per_tick_const);

    void SetEnergyMeterFunction(EnergyMeterFunction energy_meter_function) {
        energy_meter_function_ = std::move(energy_meter_function);
    }

    const auto& GetComponents() const { return components_; }

    ~IResource() override = default;

 protected:
    /**
     * This class is abstract, constructor can be called only from derived
     */
    explicit IResource(std::string type) : events::IActor(std::move(type)) {}

    /**
     * Set of IResources, which are components of this IResource (e.g.,
     * data-centers are components of the cloud).
     *
     * Is used in turning on/off event handlers
     */
    std::unordered_set<UUID> components_;

    void AddComponent(UUID uuid) { components_.insert(uuid); }

    enum class PowerState
    {
        kOff,
        kTurningOn,
        kTurningOff,
        kRunning,
        kFailure,
    };

    static std::string_view PowerStateToString(PowerState state);

    void SetPowerState(PowerState new_state);

    PowerState power_state_{PowerState::kOff};

    // event handlers
    virtual void StartBoot(const ResourceEvent* resource_event);
    virtual void StartShutdown(const ResourceEvent* resource_event);
    virtual void StartReboot(const ResourceEvent* resource_event);
    virtual void CompleteBoot(const ResourceEvent* resource_event);
    virtual void CompleteShutdown(const ResourceEvent* resource_event);

 private:
    TimeInterval startup_delay_{}, reboot_delay_{}, shutdown_delay_{};

    EnergyCount energy_per_tick_const_{};
    EnergyMeterFunction energy_meter_function_;
};

}   // namespace sim::infra
