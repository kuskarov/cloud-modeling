#include "event-loop.h"

#include "actor.h"

void
sim::events::EventLoop::Insert(sim::types::TimeStamp ts, const Event& event,
                               bool insert_to_head)
{
    if (auto it = queue_.find(ts); it != queue_.end()) {
        auto& deq = it->second;
        if (insert_to_head) {
            deq.push_front(event);
        } else {
            deq.push_back(event);
        }
    } else {
        queue_.insert({ts, {event}});
    }
}

void
sim::events::EventLoop::SimulateNextTimeStamp()
{
    if (auto it = queue_.find(current_ts_); it != queue_.end()) {
        auto& deq = it->second;

        /* using such complicated way for iteration because new events may occur
         * while simulating this timestamp, even in the front of the queue
         */
        auto deq_it = deq.begin();
        while (deq_it != deq.end()) {
            const Event& event = *deq_it;

            if (!event.is_cancelled()) {
                for (auto& addressee : event.addressees) {
                    addressee->HandleEvent(event);
                }
            }

            deq.pop_front();
            deq_it = deq.begin();
        }
    } else {
        abort();   // unreachable? check by abort()
    }
}

/**
 * Synchronous version, no suspends are supported
 *
 * TODO: add breaks (run it another thread + atomic finish_req flag?)
 */
void
sim::events::EventLoop::RunSimulation()
{
    while (!queue_.empty()) {
        auto least_ts_it = queue_.begin();
        current_ts_ = least_ts_it->first;
        SimulateNextTimeStamp();
    }
}
