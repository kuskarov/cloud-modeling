#include "manager.h"

#include <iostream>
#include <loguru.hpp>

#include "scheduler.h"
#include "types.h"
#include "vm-storage.h"
#include "vm.h"

void
sim::core::Manager::Setup()
{
    event_loop_ = std::make_shared<events::EventLoop>();
    auto schedule_callback = [this](types::TimeStamp ts,
                                    const std::shared_ptr<events::Event>& event,
                                    bool insert_to_head) {
        event_loop_->Insert(ts, event, insert_to_head);
    };

    schedule_callback_ = schedule_callback;

    cloud_ = std::make_shared<infra::Cloud>();
    cloud_->SetOwner(this);

    vm_storage_ = std::make_shared<VMStorage>();

    scheduler_ = std::make_shared<Scheduler>();

    config_->ParseResources(
        [this](std::shared_ptr<infra::DataCenter> dc) {
            dc->SetOwner(cloud_.get());
            cloud_->AddDataCenter(std::move(dc));
        },
        schedule_callback_);
}

/** First version: simple CLI loop, which can
 *
 * 1) run 1,2,...,n, all simulation steps in event-loop
 * 2) create and schedule events like CreateVM, KillVM, ...
 *
 */
void
sim::core::Manager::Listen()
{
    std::string command{};
    while (true) {
        std::getline(std::cin, command);

        if (command == "q") {
            break;
        } else if (command == "run all") {
            auto startup = std::make_shared<events::Event>();
            startup->addressee = cloud_.get();
            startup->is_cancelled = [] { return false; };

            schedule_callback_(types::TimeStamp{1}, startup, false);

            event_loop_->SimulateAll();
        }
    }
    LOG_F(INFO, "Quit!");
}

/**
 *
 * Handle user command to create VM. Algorithm:
 *
 * 1) Parse input -> infra::VM object
 * 2) Register VM object in VMStorage
 * 3) Call scheduler
 *
 */
void
sim::core::Manager::CreateVM(const std::string& command)
{
    auto vm = std::make_shared<infra::VM>();

    // TODO: parse input and setup VM

    vm_storage_->InsertVM(types::UniqueUUID(), vm);

    // schedule VMStorageEvent

    // schedule ScheduleEvent (call Scheduler)

    // notificate myself
}
