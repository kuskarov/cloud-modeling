#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "types.h"

namespace sim::events {

class IActor;

/**
 * Abstract class for an event, may contain some context in derived
 * implementations
 */
struct Event
{
    UUID id;

    TimeStamp happen_time;

    /**
     * A closure to get info about if the event was cancelled
     */
    std::function<bool()> is_cancelled = [] { return false; };

    /**
     * Actor whose HandleEvent() method should be called
     */
    UUID addressee{};

    /**
     * Event which should be scheduled after the chain of events ended
     */
    Event* notificator{};

    virtual ~Event() = default;
};

template <typename TEvent>
TEvent*
MakeEvent(UUID addressee, TimeStamp happen_time, Event* notificator)
{
    static_assert(std::is_base_of<Event, TEvent>::value);

    auto event = new TEvent();
    event->addressee = addressee;
    event->happen_time = happen_time;
    event->notificator = notificator;

    return event;
}

template <typename TEvent>
TEvent*
MakeInheritedEvent(UUID addressee, const Event* base_event, TimeInterval delay)
{
    static_assert(std::is_base_of<Event, TEvent>::value);

    auto event = new TEvent();
    event->addressee = addressee;
    event->happen_time = base_event->happen_time + delay;
    event->notificator = base_event->notificator;

    return event;
}

}   // namespace sim::events
