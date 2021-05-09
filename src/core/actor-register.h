#pragma once

#include <memory>
#include <unordered_map>

#include "actor.h"
#include "types.h"

namespace sim::core {

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
    const Actor* GetActor(types::UUID uuid) const
    {
        static_assert(std::is_base_of<events::IActor, Actor>::value);

        return dynamic_cast<const Actor*>(actors_.at(uuid).get());
    }

    template <class Actor>
    Actor* GetActor(types::UUID uuid)
    {
        static_assert(std::is_base_of<events::IActor, Actor>::value);

        return dynamic_cast<Actor*>(actors_.at(uuid).get());
    }

    types::UUID GetActorHandle(const std::string& name)
    {
        return actors_names_.at(name);
    }

    template <class Actor>
    types::UUID Make(std::string name)
    {
        static_assert(std::is_base_of<events::IActor, Actor>::value);

        auto actor = new Actor();
        actor->SetScheduleFunction(schedule_function_);

        actors_.emplace(actor->UUID(), actor);

        actor->SetName(name);
        if (auto it = actors_names_.find(name); it != actors_names_.end()) {
            throw std::logic_error("Name is not unique");
        }

        actors_names_[name] = actor->UUID();

        CORE_LOG_INFO("Registered Actor {} with name {}", actor->UUID(),
                      actor->GetName());

        return actor->UUID();
    }

    void SetScheduleFunction(const events::ScheduleFunction& sf)
    {
        schedule_function_ = sf;
    }

 private:
    std::string whoami_{};

    events::ScheduleFunction schedule_function_;

    std::unordered_map<std::string, types::UUID> actors_names_;
    std::unordered_map<types::UUID, std::unique_ptr<events::IActor>> actors_;
};

}   // namespace sim::core
