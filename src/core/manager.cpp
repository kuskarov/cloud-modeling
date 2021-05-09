#include "manager.h"

#include <iostream>
#include <sstream>

#include "logger.h"
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

    event_loop_ = std::make_unique<events::EventLoop>();

    schedule_event = [this](events::Event* event, bool immediate) {
        event_loop_->Insert(event, immediate);
    };

    actor_register_ = std::make_unique<ActorRegister>();
    actor_register_->SetScheduleFunction(schedule_event);

    event_loop_->SetActorFromUUIDCallback(
        [this](types::UUID uuid) -> events::IActor* {
            return actor_register_->GetActor<events::IActor>(uuid);
        });

    cloud_handle = actor_register_->Make<infra::Cloud>("cloud-1");

    auto cloud = actor_register_->GetActor<infra::Cloud>(cloud_handle);

    vm_storage_handle = actor_register_->Make<infra::VMStorage>("vm-storage-1");
    cloud->SetVMStorage(vm_storage_handle);

    scheduler_handle =
        actor_register_->Make<FirstAvailableScheduler>("scheduler-1");
    auto scheduler_ =
        actor_register_->GetActor<FirstAvailableScheduler>(scheduler_handle);
    scheduler_->SetActorRegister(actor_register_.get());
    scheduler_->SetCloud(cloud_handle);

    config_->ParseResources(cloud_handle, actor_register_.get());
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
    // ACTOR_LOG_INFO("Quit!");
}

sim::types::UUID
sim::core::Manager::ResolveNameFromInput(const std::string& command)
{
    std::istringstream ss(command);
    std::string _, name;
    ss >> _ >> name;

    types::UUID resource_uuid;
    try {
        resource_uuid = actor_register_->GetActorHandle(name);
        return resource_uuid;
    } catch (...) {
        CORE_LOG_ERROR("Manager", "Name {} not found", name);
        return types::NoneUUID();
    }
}

void
sim::core::Manager::Boot(const std::string& command)
{
    if (auto resource_uuid = ResolveNameFromInput(command);
        resource_uuid != types::NoneUUID()) {
        auto boot_event = events::MakeEvent<infra::ResourceEvent>(
            resource_uuid, event_loop_->Now(), nullptr);
        boot_event->type = infra::ResourceEventType::kBoot;

        schedule_event(boot_event, false);
    }
}

void
sim::core::Manager::Shutdown(const std::string& command)
{
    if (auto resource_uuid = ResolveNameFromInput(command);
        resource_uuid != types::NoneUUID()) {
        auto shutdown_event = events::MakeEvent<infra::ResourceEvent>(
            resource_uuid, event_loop_->Now(), nullptr);
        shutdown_event->type = infra::ResourceEventType::kShutdown;

        schedule_event(shutdown_event, false);
    }
}

void
sim::core::Manager::CreateVM(const std::string& command)
{
    std::istringstream ss(command);
    std::string _, vm_name;
    ss >> _ >> vm_name;

    types::UUID vm_uuid{};
    try {
        vm_uuid = actor_register_->Make<infra::VM>(vm_name);
    } catch (const std::logic_error& le) {
        CORE_LOG_ERROR("Manager", "Name {} is not unique", vm_name);
        return;
    }

    auto vm_storage_event = events::MakeEvent<infra::VMStorageEvent>(
        vm_storage_handle, event_loop_->Now(), nullptr);
    vm_storage_event->type = infra::VMStorageEventType::kVMCreated;
    vm_storage_event->uuid = vm_uuid;

    schedule_event(vm_storage_event, false);
}

void
sim::core::Manager::ProvisionVM(const std::string& command)
{
    if (auto vm_uuid = ResolveNameFromInput(command);
        vm_uuid != types::NoneUUID()) {
        auto vm_storage_event = events::MakeEvent<infra::VMStorageEvent>(
            vm_storage_handle, event_loop_->Now(), nullptr);
        vm_storage_event->type =
            infra::VMStorageEventType::kVMProvisionRequested;
        vm_storage_event->uuid = vm_uuid;
        schedule_event(vm_storage_event, true);

        // event that should happen after the chain of events ends
        auto vm_storage_notification = new infra::VMStorageEvent();
        vm_storage_notification->type =
            infra::VMStorageEventType::kVMProvisioned;
        vm_storage_notification->addressee = vm_storage_handle;
        vm_storage_notification->uuid = vm_uuid;

        auto scheduler_event = events::MakeEvent<SchedulerEvent>(
            scheduler_handle, event_loop_->Now(), vm_storage_notification);
        scheduler_event->type = SchedulerEventType::kProvisionVM;

        schedule_event(scheduler_event, false);
    }
}

void
sim::core::Manager::StopVM(const std::string& command)
{
    if (auto vm_uuid = ResolveNameFromInput(command);
        vm_uuid != types::NoneUUID()) {
        // event that should happen after the chain of events ends
        auto vm_storage_notification = new infra::VMStorageEvent();
        vm_storage_notification->type = infra::VMStorageEventType::kVMStopped;
        vm_storage_notification->addressee = vm_storage_handle;
        vm_storage_notification->uuid = vm_uuid;

        auto stop_vm_event = events::MakeEvent<infra::VMEvent>(
            vm_uuid, event_loop_->Now(), vm_storage_notification);
        stop_vm_event->type = infra::VMEventType::kStop;

        schedule_event(stop_vm_event, false);
    }
}

void
sim::core::Manager::DeleteVM(const std::string& command)
{
    if (auto vm_uuid = ResolveNameFromInput(command);
        vm_uuid != types::NoneUUID()) {
        // event that should happen after the chain of events ends
        auto vm_storage_notification = new infra::VMStorageEvent();
        vm_storage_notification->type = infra::VMStorageEventType::kVMDeleted;
        vm_storage_notification->addressee = vm_storage_handle;
        vm_storage_notification->uuid = vm_uuid;

        auto delete_vm_event = events::MakeEvent<infra::VMEvent>(
            vm_uuid, event_loop_->Now(), vm_storage_notification);
        delete_vm_event->type = infra::VMEventType::kDelete;

        schedule_event(delete_vm_event, false);
    }
}
