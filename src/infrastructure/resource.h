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
class IResource : public events::IActor
{
 public:
    explicit IResource(std::string type) : events::IActor(std::move(type)) {}

    void HandleEvent(const events::Event* event) override;

    virtual types::EnergyCount SpentPower() = 0;

    [[nodiscard]] types::TimeInterval GetStartupDelay() const;
    void SetStartupDelay(types::TimeInterval startup_delay);
    [[nodiscard]] types::TimeInterval GetRebootDelay() const;
    void SetRebootDelay(types::TimeInterval reboot_delay);
    [[nodiscard]] types::TimeInterval GetShutdownDelay() const;
    void SetShutdownDelay(types::TimeInterval shutdown_delay);

    void SetName(std::string name) override;

    ~IResource() override = default;

 protected:
    inline bool PowerStateIs(ResourcePowerState expected,
                             const std::string& caller_info);

    ResourcePowerState power_state_{ResourcePowerState::kOff};

    // TODO: remove magic numbers
    types::TimeInterval startup_delay_{3}, reboot_delay_{4}, shutdown_delay_{1};

    // event handlers
    virtual void StartBoot(const ResourceEvent* resource_event);
    virtual void StartShutdown(const ResourceEvent* resource_event);
    virtual void StartReboot(const ResourceEvent* resource_event);
    virtual void CompleteBoot(const ResourceEvent* resource_event);
    virtual void CompleteShutdown(const ResourceEvent* resource_event);
};

/**
 * Class for convenient way to create a colony of identical IResource instances
 * from the template
 *
 */
template <class Resource>
class ResourceGenerator
{
 public:
    explicit ResourceGenerator(Resource&& resource_template)
        : resource_template_(resource_template)
    {
        static_assert(std::is_base_of<IResource, Resource>::value);
    }

    void SetOwner(events::IActor* owner) { resource_template_.SetOwner(owner); }

    void SetScheduleFunction(const events::ScheduleFunction& function)
    {
        resource_template_.SetScheduleFunction(function);
    }

    std::shared_ptr<Resource> operator()()
    {
        auto resource = std::make_shared<Resource>(resource_template_);
        resource->SetName(resource_template_.GetName() + "-" +
                          std::to_string(++serial));

        return resource;
    }

 private:
    Resource resource_template_;
    uint32_t serial{};
};

/**
 * Only to avoid passing generator by value to std::generate_n
 *
 */
template <class Resource>
class ResourceGeneratorWrapper
{
 public:
    explicit ResourceGeneratorWrapper(ResourceGenerator<Resource>& generator)
        : generator_(&generator)
    {
    }

    std::shared_ptr<Resource> operator()() { return (*generator_)(); }

 private:
    ResourceGenerator<Resource>* generator_{};
};

}   // namespace sim::infra
