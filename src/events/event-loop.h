#pragma once

#include <deque>
#include <map>

#include "event.h"

namespace sim::events {

/**
Using deque to have ability of adding event to the head or to the end of queue
 */
typedef std::map<types::TimeStamp, std::deque<Event>> EventQueue;

class EventLoop
{
 public:
    EventLoop() = default;

    /**
     * To be used in closure passed to each component able to generate events
     */
    void Insert(types::TimeStamp ts, const Event& event,
                bool insert_to_head = false);

    void SimulateNextTimeStamp();

    void RunSimulation();

 private:
    types::TimeStamp current_ts_{};
    EventQueue queue_{};
};

}   // namespace sim::events
