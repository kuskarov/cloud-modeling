#pragma once

#include <functional>
#include <loguru.hpp>
#include <string>

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
    Actor() = default;

    virtual void HandleEvent(const std::shared_ptr<Event>& event)
    {
        throw std::logic_error(
            "abstract class method invocation! not implemented");
    }

    void SetScheduleCallback(
        const std::function<void(types::TimeStamp,
                                 const std::shared_ptr<Event>&, bool)>&
            schedule_callback)
    {
        schedule_callback_ = schedule_callback;
    }

    [[nodiscard]] const std::string& GetName() const { return name_; }
    void SetName(const std::string& name) { name_ = name; }

    virtual ~Actor() { LOG_F(INFO, "Destroying actor %s", name_.c_str()); }

 protected:
    std::function<void(types::TimeStamp, const std::shared_ptr<Event>&, bool)>
        schedule_callback_;

    std::string name_{};
};

}   // namespace sim::events
