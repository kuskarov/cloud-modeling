#include "manager.h"

#include <iostream>
#include <sstream>

#include "logger.h"
#include "resource-register.h"
#include "scheduler.h"
#include "types.h"
#include "vm-storage.h"
#include "vm.h"

void
sim::core::Manager::Setup()
{
    SimulatorLogger::SetCSVFolder("../../../");
    SimulatorLogger::SetMaxCSVSeverity(LogSeverity::kDebug);
    SimulatorLogger::SetMaxConsoleSeverity(LogSeverity::kDebug);
    SimulatorLogger::SetTimeCallback([this] { return event_loop_->Now(); });

    event_loop_ = std::make_shared<events::EventLoop>();
    schedule_event = [this](events::Event* event, bool immediate) {
        event_loop_->Insert(event, immediate);
    };

    register_ = std::make_shared<infra::ResourceRegister>();

    cloud_ = std::make_shared<infra::Cloud>();
    cloud_->SetOwner(this);
    cloud_->SetScheduleFunction(schedule_event);

    vm_storage_ = std::make_shared<infra::VMStorage>();
    vm_storage_->SetOwner(this);
    vm_storage_->SetScheduleFunction(schedule_event);

    scheduler_ = std::make_shared<FirstAvailableScheduler>(cloud_, vm_storage_);
    scheduler_->SetOwner(this);
    scheduler_->SetScheduleFunction(schedule_event);

    config_->ParseResources(cloud_, vm_storage_);

    // TODO: remove this, add new abstractions
    for (auto& dc : cloud_->DataCenters()) {
        for (auto& server : dc->Servers()) {
            server->SetVMStorage(vm_storage_.get());
        }
    }
}

/** First version: simple CLI loop, which can
 *
 * 1) run 1,2,...,n, all simulation steps in event-loop
 * 2) create and schedule events like CreateVM, DeleteVM, ...
 *
 */
void
sim::core::Manager::Listen()
{
    std::cerr
        << "\n"
           "Temporal CLI:\n"
           "boot <name>                 : boot resource <name>\n"
           "shutdown <name>             : shutdown resource <name>\n"
           "create-vm <vm-name> <ram>   : create (but do not provision) vm\n"
           "provision-vm <vm-name>      : provision and run vm <vm-name>\n"
           "stop-vm <vm-name>           : stop (but do not delete) vm "
           "<vm-name>\n"
           "delete-vm <vm-name>         : delete vm <vm-name>\n"
           "q                           : quit\n";

    std::string command{};
    while (true) {
        std::getline(std::cin, command);

        if (command == "q") {
            break;
        } else if (command.find("boot") != std::string::npos) {
            Boot(command);
        } else if (command.find("shutdown") != std::string::npos) {
            Shutdown(command);
        } else if (command.find("create-vm") != std::string::npos) {
            CreateVM(command);
        } else if (command.find("provision-vm") != std::string::npos) {
            ProvisionVM(command);
        } else if (command.find("stop-vm") != std::string::npos) {
            StopVM(command);
        } else if (command.find("delete-vm") != std::string::npos) {
            DeleteVM(command);
        }
        event_loop_->SimulateAll();
    }
    ACTOR_LOG_INFO("Quit!");
}

void
sim::core::Manager::Boot(const std::string& command)
{
    std::istringstream ss(command);
    std::string _, resource_name;
    ss >> _ >> resource_name;

    auto boot_event = new infra::ResourceEvent();

    boot_event->addressee = register_->GetResourceHandle(resource_name);
    boot_event->type = infra::ResourceEventType::kBoot;
    boot_event->happen_time = event_loop_->Now();

    schedule_event(boot_event, false);
}

void
sim::core::Manager::Shutdown(const std::string& command)
{
    std::istringstream ss(command);
    std::string _, resource_name;
    ss >> _ >> resource_name;

    auto shutdown = new infra::ResourceEvent();

    shutdown->addressee = register_->GetResourceHandle(resource_name);
    shutdown->type = infra::ResourceEventType::kShutdown;
    shutdown->happen_time = event_loop_->Now();

    schedule_event(shutdown, false);
}

void
sim::core::Manager::CreateVM(const std::string& command)
{
    auto vm = std::make_shared<infra::VM>();
    vm->SetScheduleFunction(schedule_event);

    std::istringstream ss(command);
    std::string _, vm_name;
    types::RAMBytes ram;
    ss >> _ >> vm_name >> ram;

    vm->SetName(vm_name);
    vm->SetRam(ram);

    vm_storage_->InsertVM(vm);

    auto vm_storage_event = new infra::VMStorageEvent();
    vm_storage_event->type = infra::VMStorageEventType::kVMCreated;
    vm_storage_event->addressee = vm_storage_.get();
    vm_storage_event->happen_time = event_loop_->Now();
    vm_storage_event->vm_name = vm_name;

    schedule_event(vm_storage_event, false);
}

void
sim::core::Manager::ProvisionVM(const std::string& command)
{
    std::istringstream ss(command);
    std::string _, vm_name;
    ss >> _ >> vm_name;

    // event that should happen after the chain of events ends
    auto vm_storage_notification = new infra::VMStorageEvent();
    vm_storage_notification->type = infra::VMStorageEventType::kVMProvisioned;
    vm_storage_notification->addressee = vm_storage_.get();
    vm_storage_notification->vm_name = vm_name;

    auto scheduler_event = new SchedulerEvent();
    scheduler_event->type = SchedulerEventType::kProvisionVM;
    scheduler_event->addressee = scheduler_.get();
    scheduler_event->happen_time = event_loop_->Now();
    scheduler_event->notificator = vm_storage_notification;

    schedule_event(scheduler_event, false);
}

void
sim::core::Manager::StopVM(const std::string& command)
{
    std::istringstream ss(command);
    std::string _, vm_name;
    ss >> _ >> vm_name;

    // event that should happen after the chain of events ends
    auto vm_storage_notification = new infra::VMStorageEvent();
    vm_storage_notification->type = infra::VMStorageEventType::kVMStopped;
    vm_storage_notification->addressee = vm_storage_.get();
    vm_storage_notification->vm_name = vm_name;

    auto stop_vm_event = new infra::VMEvent;
    stop_vm_event->addressee = vm_storage_->GetVM(vm_name).get();
    stop_vm_event->type = infra::VMEventType::kStop;
    stop_vm_event->happen_time = event_loop_->Now();
    stop_vm_event->notificator = vm_storage_notification;

    schedule_event(stop_vm_event, false);
}

void
sim::core::Manager::DeleteVM(const std::string& command)
{
    std::istringstream ss(command);
    std::string _, vm_name;
    ss >> _ >> vm_name;

    // event that should happen after the chain of events ends
    auto vm_storage_notification = new infra::VMStorageEvent();
    vm_storage_notification->type = infra::VMStorageEventType::kVMDeleted;
    vm_storage_notification->addressee = vm_storage_.get();
    vm_storage_notification->vm_name = vm_name;

    auto delete_vm_event = new infra::VMEvent;
    delete_vm_event->addressee = vm_storage_->GetVM(vm_name).get();
    delete_vm_event->type = infra::VMEventType::kDelete;
    delete_vm_event->happen_time = event_loop_->Now();
    delete_vm_event->notificator = vm_storage_notification;

    schedule_event(delete_vm_event, false);
}
