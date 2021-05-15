#pragma once

#include <deque>
#include <map>

#include "actor.h"
#include "event.h"

namespace sim::events {

/**
Using deque to have ability of adding event to the head or to the end of queue
 */
typedef std::map<TimeStamp, std::deque<std::shared_ptr<Event>>> EventQueue;

class EventLoop
{
 public:
    EventLoop() : whoami_("Event-Loop") {}

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
    [[nodiscard]] TimeStamp Now() const { return current_ts_; }

    void SetActorFromUUIDCallback(const std::function<IActor*(UUID)>& cb)
    {
        actor_from_uuid = cb;
    }

    void SetUpdateWorldCallback(const std::function<void()>& cb)
    {
        update_world = cb;
    }

 private:
    const std::string whoami_{};

    void SimulateNextStep();

    std::function<IActor*(UUID)> actor_from_uuid;

    std::function<void()> update_world;

    TimeStamp current_ts_{1};
    EventQueue queue_{};
};

}   // namespace sim::events
