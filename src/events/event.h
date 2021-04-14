#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "types.h"

namespace sim::events {
class Actor;

/**
 * Abstract struct for an event, may contain some context in derived
 * implementations
 */
struct Event
{
    // TODO: are they needed?
    sim::types::UUID uuid;
    sim::types::TimeStamp creation_ts, happen_ts;
    virtual void do_nothing() {}

    /// a closure to get info about if the event cancelled before the time of
    /// handling
    std::function<bool()> is_cancelled;

    // TODO: generate unique event for each addressee?
    std::vector<std::shared_ptr<Actor>> addressees;
};

}   // namespace sim::events
