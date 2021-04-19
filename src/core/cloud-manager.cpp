#include "cloud-manager.h"

#include <iostream>
#include <loguru.hpp>

void
sim::core::CloudManager::Setup()
{
    config_->ParseResources([this](std::shared_ptr<resources::DataCenter> dc) {
        data_centers_.push_back(std::move(dc));
    });

    event_loop_ = std::make_shared<events::EventLoop>();
    auto schedule_callback = [this](types::TimeStamp ts,
                                    const std::shared_ptr<events::Event>& event,
                                    bool insert_to_head) {
        event_loop_->Insert(ts, event, insert_to_head);
    };

    schedule_callback_ = schedule_callback;

    for (auto& data_center : data_centers_) {
        data_center->SetOwner(this);
        data_center->SetScheduleCallback(schedule_callback);
        data_center->SetServerScheduleCallback(schedule_callback);
    }
}

/** First version: simple CLI loop, which can
 *
 * 1) run 1,2,...,n, all simulation steps in event-loop
 * 2) create and schedule events like ProvisionVM, KillVM, ...
 *
 */
void
sim::core::CloudManager::RunCloud()
{
    std::string command{};
    while (true) {
        std::getline(std::cin, command);

        if (command == "q") {
            break;
        } else if (command == "run all") {
            auto startup = std::make_shared<events::Event>();
            startup->addressee = (*data_centers_.begin()).get();
            startup->is_cancelled = [] { return false; };

            schedule_callback_(types::TimeStamp{1}, startup, false);

            event_loop_->SimulateAll();
        }
    }
    LOG_F(INFO, "Quit!");
}

void
sim::core::CloudManager::ProvisionVM()
{
}
