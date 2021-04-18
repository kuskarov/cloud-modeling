#include "event-loop.h"

#include <loguru.hpp>

#include "actor.h"

void
sim::events::EventLoop::Insert(sim::types::TimeStamp ts,
                               const std::shared_ptr<Event>& event,
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
sim::events::EventLoop::SimulateAll()
{
    while (true) {
        if (queue_.empty()) {
            break;
        }
        SimulateNextStep();
    }
}

void
sim::events::EventLoop::SimulateUntil(uint32_t until_ts)
{
    while (true) {
        if (current_ts_ > until_ts) {
            break;
        }
        SimulateNextStep();
    }
}

void
sim::events::EventLoop::SimulateSteps(uint32_t steps_count)
{
    for (uint32_t i = 0; i < steps_count; ++i) {
        SimulateNextStep();
    }
}

void
sim::events::EventLoop::SimulateNextStep()
{
    LOG_F(INFO, "Simulating step!");
    if (queue_.empty()) {
        LOG_F(INFO, "Queue is empty!");
    } else {
        auto& [ts, ts_queue] = *queue_.begin();

        current_ts_ = ts;

        // ts_queue cannot be empty
        auto event = *ts_queue.begin();
        if (!event->is_cancelled()) {
            event->addressee->HandleEvent(event);
        } else {
            LOG_F(INFO, "Event %lu was not called because it was cancelled",
                  event->id);
        }

        LOG_F(INFO, "Event: ts = %lu", event->happen_ts);

        ts_queue.pop_front();
        if (ts_queue.empty()) {
            queue_.erase(ts);
        }
    }
}
