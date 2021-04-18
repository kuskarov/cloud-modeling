#pragma once

#include <deque>
#include <map>

#include "event.h"

namespace sim::events {

/**
Using deque to have ability of adding event to the head or to the end of queue
 */
typedef std::map<types::TimeStamp, std::deque<std::shared_ptr<Event>>>
    EventQueue;

class EventLoop
{
 public:
    /**
     *
     * To be used in closure passed to each component able to generate events
     */
    void Insert(types::TimeStamp ts, const std::shared_ptr<Event>& event,
                bool insert_to_head = false);

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

 private:
    void SimulateNextStep();

    types::TimeStamp current_ts_{};
    EventQueue queue_{};
};

}   // namespace sim::events
