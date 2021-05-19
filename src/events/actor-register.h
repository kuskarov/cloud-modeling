#pragma once

#include <memory>
#include <unordered_map>

#include "actor.h"
#include "types.h"

namespace sim::events {

/**
 * This class is responsible for owning all IActor objects in the simulator.
 *
 * It gives methods for creating new IActor instance and getting pointer from
 * UUID
 */
class ActorRegister
{
 public:
    ActorRegister() : whoami_("Actor-Register") {}

    /**
     * Const version for IScheduler
     */
    template <class Actor>
    const Actor* GetActor(UUID uuid) const
    {
        static_assert(std::is_base_of_v<events::IActor, Actor>);

        return dynamic_cast<const Actor*>(actors_.at(uuid).get());
    }

    template <class Actor>
    Actor* GetActor(UUID uuid)
    {
        static_assert(std::is_base_of_v<events::IActor, Actor>);

        return dynamic_cast<Actor*>(actors_.at(uuid).get());
    }

    UUID GetActorHandle(const std::string& name)
    {
        return actors_names_.at(name);
    }

    template <class Actor>
    Actor* Make(std::string name)
    {
        static_assert(std::is_base_of_v<events::IActor, Actor>);

        auto actor = new Actor();
        actor->SetScheduleFunction(schedule_function_);

        actors_.emplace(actor->GetUUID(), actor);

        actor->SetName(name);
        if (auto it = actors_names_.find(name); it != actors_names_.end()) {
            throw std::logic_error(fmt::format("Name {} is not unique", name));
        }

        actors_names_[name] = actor->GetUUID();

        WORLD_LOG_INFO("Registered Actor {} with name {}", actor->GetUUID(),
                       actor->GetName());

        return actor;
    }

    void SetScheduleFunction(const events::ScheduleFunction& sf)
    {
        schedule_function_ = sf;
    }

 private:
    std::string whoami_{};

    events::ScheduleFunction schedule_function_;

    std::unordered_map<std::string, UUID> actors_names_;
    std::unordered_map<UUID, std::unique_ptr<events::IActor>> actors_;
};

}   // namespace sim::events
