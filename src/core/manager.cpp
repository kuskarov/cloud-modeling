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
    cloud_->SetScheduleCallback(schedule_callback_);

    vm_storage_ = std::make_shared<VMStorage>();
    vm_storage_->SetOwner(this);
    vm_storage_->SetScheduleCallback(schedule_callback_);

    scheduler_ = std::make_shared<Scheduler>(cloud_, vm_storage_);
    scheduler_->SetOwner(this);
    scheduler_->SetScheduleCallback(schedule_callback_);

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
            auto startup = std::make_shared<infra::ResourceEvent>();
            startup->addressee = cloud_.get();
            startup->resource_event_type = infra::ResourceEventType::kBoot;
            startup->happen_ts = types::TimeStamp{1};

            schedule_callback_(types::TimeStamp{1}, startup, false);
        } else if (command == "create") {
            CreateVM(command);
        } else if (command == "provision") {
            Provision(command);
        }
        event_loop_->SimulateAll();
    }
    LOG_F(INFO, "Quit!");
}

void
sim::core::Manager::CreateVM(const std::string& command)
{
    auto vm = std::make_shared<infra::VM>();
    vm->SetScheduleCallback(schedule_callback_);

    // TODO: parse input and setup VM

    types::UUID uuid = types::GenerateUUID();
    vm_storage_->InsertVM(uuid, vm);

    auto vm_storage_event = std::make_shared<VMStorageEvent>();
    vm_storage_event->type = VMStorageEventType::kVMCreated;
    vm_storage_event->addressee = vm_storage_.get();
    vm_storage_event->happen_ts = event_loop_->Now();
    vm_storage_event->vm_uuid = uuid;

    schedule_callback_(event_loop_->Now(), vm_storage_event, true);
}

void
sim::core::Manager::Provision(const std::string& command)
{
    // event that should happen after the chain of events ends
    auto vm_storage_notification = std::make_shared<VMStorageEvent>();
    vm_storage_notification->type = VMStorageEventType::kVMProvisioned;
    vm_storage_notification->addressee = vm_storage_.get();

    auto scheduler_event = std::make_shared<SchedulerEvent>();
    scheduler_event->type = SchedulerEventType::kProvisionVM;
    scheduler_event->addressee = scheduler_.get();
    scheduler_event->happen_ts = event_loop_->Now();
    schedule_callback_(event_loop_->Now(), scheduler_event, true);
}
