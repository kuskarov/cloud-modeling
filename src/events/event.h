#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "types.h"

namespace sim::events {

class Actor;

/**
 * Abstract class for an event, may contain some context in derived
 * implementations
 */
struct Event
{
    types::UUID id;

    types::TimeStamp creation_ts, happen_ts;

    /**
     * A closure to get info about if the event cancelled before the time of
     * handling
     */
    std::function<bool()> is_cancelled = [] { return false; };

    /**
     * Actor which HandleEvent() method should be called
     */
    Actor* addressee{};

    /**
     * Event which should be scheduled after the chain of events ended
     *
     */
    std::shared_ptr<Event> notificator{};

    virtual ~Event() = default;
};

}   // namespace sim::events
