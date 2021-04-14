#include "cloud-manager.h"

bool
sim::core::CloudManager::Setup()
{
    if (!config_->ParseResources([this](resources::DataCenter &&dc) {
            data_centers_.emplace_back(dc);
        })) {
        return false;
    }

    /*
    if (!config_->ParseTasks()) {
        return false;
    }
    */

    event_loop_ = std::make_shared<events::EventLoop>();
    auto schedule_callback = [this](types::TimeStamp ts,
                                    const events::Event &event,
                                    bool insert_to_head) {
        event_loop_->Insert(ts, event, insert_to_head);
    };

    for (auto &data_center : data_centers_) {
        data_center.SetScheduleCallback(schedule_callback);
        data_center.SetServerScheduleCallback(schedule_callback);
    }

    return true;
}

void
sim::core::CloudManager::RunCloud()
{
    event_loop_->RunSimulation();
}
