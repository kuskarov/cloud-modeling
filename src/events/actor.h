#pragma once

#include <functional>
#include <loguru.hpp>
#include <string>
#include <utility>

#include "event.h"
#include "types.h"

namespace sim::events {
/**
 * Abstract class for Actor. Each Actor should be able to HandleEvent (may
 * handle own overridden event) and has a callback for scheduling events
 */
class Actor
{
 public:
    explicit Actor(std::string&& type) : type_(type) {}

    virtual void HandleEvent(const std::shared_ptr<Event>& event) = 0;

    void SetScheduleCallback(
        const std::function<void(types::TimeStamp,
                                 const std::shared_ptr<Event>&, bool)>&
            schedule_callback)
    {
        schedule_callback_ = schedule_callback;
    }

    void SetOwner(Actor* owner) { owner_ = owner; }

    [[nodiscard]] const std::string& GetName() const { return name_; }
    void SetName(const std::string& name) { name_ = name; }

    [[nodiscard]] const std::string& GetType() const { return type_; }
    void SetType(const std::string& type) { type_ = type; }

    virtual ~Actor() = default;

 protected:
    std::function<void(types::TimeStamp, const std::shared_ptr<Event>&, bool)>
        schedule_callback_;

    std::string type_{"Actor"}, name_{"Unnamed"};

    Actor* owner_{};
};

}   // namespace sim::events
