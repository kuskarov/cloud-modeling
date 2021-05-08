#pragma once

#include <deque>
#include <map>

#include "actor.h"
#include "event.h"

namespace sim::events {

/**
Using deque to have ability of adding event to the head or to the end of queue
 */
typedef std::map<types::TimeStamp, std::deque<std::shared_ptr<Event>>>
    EventQueue;

class EventLoop : public IActor
{
 public:
    EventLoop() : IActor("Event-Loop") { SetName("Event-Loop"); }

    /**
     *
     * To be used in closure passed to each component able to generate events
     */
    void Insert(Event* event, bool immediate = false);

    /**
     *
     * @param steps_count Count of steps to simulate
     */
    void SimulateSteps(uint32_t steps_count);

    void SimulateUntil(uint32_t until_ts);

    /**
     * Simulate until the queue is empty
     */
    void SimulateAll();

    /**
     * Available only inside the CloudManager
     *
     * @return real time at the moment of call
     */
    [[nodiscard]] types::TimeStamp Now() const { return current_ts_; }

    void HandleEvent(const events::Event* event) override {}

    void SetActorFromUUIDCallback(const std::function<IActor*(types::UUID)>& cb)
    {
        actor_from_uuid = cb;
    }

 private:
    void SimulateNextStep();

    std::function<IActor*(types::UUID)> actor_from_uuid;

    types::TimeStamp current_ts_{1};
    EventQueue queue_{};
};

}   // namespace sim::events
