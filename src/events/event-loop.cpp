#include "event-loop.h"

#include <memory>

#include "actor.h"
#include "logger.h"

void
sim::events::EventLoop::Insert(Event* event, bool immediate)
{
    if (!event) {
        throw std::runtime_error("Tried to schedule null event");
    }

    if (event->happen_time < current_ts_) {
        ACTOR_LOG_ERROR("Timestamp in the past!");
        return;
    }

    if (auto it = queue_.find(event->happen_time); it != queue_.end()) {
        auto& deq = it->second;
        if (immediate) {
            deq.emplace_front(event);
        } else {
            deq.emplace_back(event);
        }
    } else {
        queue_[event->happen_time].emplace_back(event);
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
    if (queue_.empty()) {
        ACTOR_LOG_INFO("Queue is empty!");
    } else {
        auto& [ts, ts_queue] = *queue_.begin();

        current_ts_ = ts;

        // ts_queue cannot be empty
        auto event = (*ts_queue.begin());
        ts_queue.pop_front();

        if (!event->is_cancelled()) {
            event->addressee->HandleEvent(event.get());
        } else {
            ACTOR_LOG_INFO("Event {} was not called because it was cancelled",
                           event->id);
        }

        if (ts_queue.empty()) {
            queue_.erase(ts);
        }
    }
}
