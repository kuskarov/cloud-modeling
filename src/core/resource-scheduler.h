#pragma once

#include <utility>

#include "observer.h"
#include "resource.h"
#include "server.h"
#include "types.h"
#include "vm.h"

namespace sim::core {

template <class Resource, class Consumer>
class IResourceScheduler : public events::Observer
{
 public:
    explicit IResourceScheduler(std::string name, UUID shared_resource)
        : events::Observer("Resource-Scheduler"),
          name_(std::move(name)),
          shared_resource_(shared_resource)
    {
        static_assert(std::is_base_of_v<infra::IResource, Resource>);
        // static_assert(std::is_base_of_v<infra::IConsumer, Consumer>);
    }

    IResourceScheduler(const IResourceScheduler& other) = delete;

    virtual void UpdateSchedule() = 0;

    auto GetName() const { return name_; }

 protected:
    const std::string name_;

    UUID shared_resource_;

    // may contain any extra state if needed
};

using IServerScheduler = IResourceScheduler<infra::Server, infra::VM>;

class ServerSchedulerManager : public events::Observer
{
 public:
    ServerSchedulerManager() : events::Observer("Server-Scheduler-Manager") {}

    void ScheduleAll()
    {
        for (auto& scheduler : schedulers_) {
            scheduler->UpdateSchedule();
        }
    }

    template <class ServerScheduler>
    void Make(UUID server_handle)
    {
        static_assert(std::is_base_of_v<IServerScheduler, ServerScheduler>);

        auto scheduler = std::make_unique<ServerScheduler>(server_handle);

        scheduler->SetScheduleFunction(schedule_event);
        scheduler->SetActorRegister(actor_register_);
        scheduler->SetNowFunction(now);

        WORLD_LOG_INFO("Server {} is scheduled using \"{}\" strategy",
                       server_handle, scheduler->GetName());

        schedulers_.push_back(std::move(scheduler));
    }

 private:
    std::vector<std::unique_ptr<IServerScheduler>> schedulers_;
};

}   // namespace sim::core
