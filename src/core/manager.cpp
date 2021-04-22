#include "manager.h"

#include <iostream>

#include "logger.h"
#include "scheduler.h"
#include "types.h"
#include "vm-storage.h"
#include "vm.h"

void
sim::core::Manager::Setup()
{
    SimulatorLogger::EnableCSVLogging("log.csv");
    SimulatorLogger::SetTimeCallback([this] { return event_loop_->Now(); });
    SimulatorLogger::Setup();

    event_loop_ = std::make_shared<events::EventLoop>();
    schedule_event = [this](events::Event* event, bool immediate) {
        event_loop_->Insert(event, immediate);
    };

    cloud_ = std::make_shared<infra::Cloud>();
    cloud_->SetOwner(this);
    cloud_->SetScheduleFunction(schedule_event);

    vm_storage_ = std::make_shared<VMStorage>();
    vm_storage_->SetOwner(this);
    vm_storage_->SetScheduleFunction(schedule_event);

    scheduler_ = std::make_shared<FirstAvailableScheduler>(cloud_, vm_storage_);
    scheduler_->SetOwner(this);
    scheduler_->SetScheduleFunction(schedule_event);

    config_->ParseResources(cloud_);
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
            auto startup = new infra::ResourceEvent();
            startup->addressee = cloud_.get();
            startup->type = infra::ResourceEventType::kBoot;
            startup->happen_time = types::TimeStamp{1};

            schedule_event(startup, false);
        } else if (command == "create") {
            CreateVM(command);
        } else if (command == "provision") {
            Provision(command);
        }
        event_loop_->SimulateAll();
    }
    ACTOR_LOG_INFO("Quit!");
}

void
sim::core::Manager::CreateVM(const std::string& command)
{
    auto vm = std::make_shared<infra::VM>();
    vm->SetScheduleFunction(schedule_event);

    // TODO: parse input and setup VM

    types::UUID uuid = types::GenerateUUID();
    vm_storage_->InsertVM(uuid, vm);

    auto vm_storage_event = new VMStorageEvent();
    vm_storage_event->type = VMStorageEventType::kVMCreated;
    vm_storage_event->addressee = vm_storage_.get();
    vm_storage_event->happen_time = event_loop_->Now();
    vm_storage_event->vm_uuid = uuid;

    schedule_event(vm_storage_event, true);
}

void
sim::core::Manager::Provision(const std::string& command)
{
    // event that should happen after the chain of events ends
    auto vm_storage_notification = std::make_shared<VMStorageEvent>();
    vm_storage_notification->type = VMStorageEventType::kVMProvisioned;
    vm_storage_notification->addressee = vm_storage_.get();

    auto scheduler_event = new SchedulerEvent();
    scheduler_event->type = SchedulerEventType::kProvisionVM;
    scheduler_event->addressee = scheduler_.get();
    scheduler_event->happen_time = event_loop_->Now();
    schedule_event(scheduler_event, true);
}
