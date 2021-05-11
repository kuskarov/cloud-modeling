#pragma once

#include <utility>

#include "resource.h"
#include "server.h"
#include "types.h"
#include "vm.h"

namespace sim::core {

template <class Resource, class Consumer>
class IResourceScheduler
{
 public:
    explicit IResourceScheduler(std::string name, types::UUID shared_resource)
        : name_(std::move(name)), shared_resource_(shared_resource)
    {
        static_assert(std::is_base_of<infra::IResource, Resource>::value);
        // static_assert(std::is_base_of<infra::IConsumer, Consumer>::value);
    }

    IResourceScheduler(const IResourceScheduler& other) = delete;

    void SetActorRegister(const ActorRegister* actor_register)
    {
        actor_register_ = actor_register;
    }

    void SetScheduleFunction(const events::ScheduleFunction& sf)
    {
        schedule_function = sf;
    }

    virtual void UpdateSchedule() = 0;

    [[nodiscard]] auto GetName() const { return name_; }

 protected:
    const std::string name_;
    const std::string whoami_{"ResourceScheduler"};

    events::ScheduleFunction schedule_function;

    const ActorRegister* actor_register_{};

    types::UUID shared_resource_;

    // may contain any extra state if needed
};

using IServerScheduler = IResourceScheduler<infra::Server, infra::VM>;



class ServerSchedulerManager
{
 public:
    ServerSchedulerManager() : whoami_("Server-Scheduler-Manager") {}

    void ScheduleAll()
    {
        for (auto& scheduler : schedulers_) {
            scheduler->UpdateSchedule();
        }
    }

    template <class ServerScheduler>
    void Make(types::UUID server_handle)
    {
        static_assert(
            std::is_base_of<IServerScheduler, ServerScheduler>::value);

        auto scheduler = std::make_unique<ServerScheduler>(server_handle);

        scheduler->SetScheduleFunction(schedule_function_);
        scheduler->SetActorRegister(actor_register_);

        WORLD_LOG_INFO("Server {} is scheduled using \"{}\" strategy",
                       server_handle, scheduler->GetName());

        schedulers_.push_back(std::move(scheduler));
    }

    void SetScheduleFunction(const events::ScheduleFunction& sf)
    {
        schedule_function_ = sf;
    }

    void SetActorRegister(const ActorRegister* actor_register)
    {
        actor_register_ = actor_register;
    }

 private:
    std::string whoami_{};

    events::ScheduleFunction schedule_function_;
    const ActorRegister* actor_register_{};

    std::vector<std::unique_ptr<IServerScheduler>> schedulers_;
};

}   // namespace sim::core
