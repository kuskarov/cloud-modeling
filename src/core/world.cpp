#include "world.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "custom-code.h"
#include "logger.h"
#include "scheduler.h"
#include "types.h"
#include "vm-storage.h"
#include "vm.h"

void
sim::core::World::Setup()
{
    SimulatorLogger::GetLogger().SetCSVFolder(config_->GetLogsPath());
    SimulatorLogger::GetLogger().SetMaxCSVSeverity(LogSeverity::kDebug);
    SimulatorLogger::GetLogger().SetMaxConsoleSeverity(LogSeverity::kDebug);

    event_loop_ = std::make_unique<events::EventLoop>();
    auto now = [this] { return event_loop_->Now(); };
    schedule_event = [this](events::Event* event, bool immediate) {
        event_loop_->Insert(event, immediate);
    };

    SimulatorLogger::GetLogger().SetTimeCallback(now);

    actor_register_ = std::make_unique<events::ActorRegister>();
    actor_register_->SetScheduleFunction(schedule_event);
    actor_register_->SetNowFunction(now);

    server_scheduler_manager_ = std::make_unique<ServerSchedulerManager>();
    server_scheduler_manager_->SetScheduleFunction(schedule_event);
    server_scheduler_manager_->SetActorRegister(actor_register_.get());
    server_scheduler_manager_->SetNowFunction(now);

    event_loop_->SetActorFromUUIDCallback([this](UUID uuid) -> events::IActor* {
        return actor_register_->GetActor<events::IActor>(uuid);
    });

    auto cloud = actor_register_->Make<infra::Cloud>("cloud-1");
    cloud_handle_ = cloud->GetUUID();

    auto vm_storage = actor_register_->Make<infra::VMStorage>("vm-storage-1");
    vm_storage_handle_ = vm_storage->GetUUID();

    cloud->SetVMStorage(vm_storage_handle_);

    scheduler_ = std::make_unique<custom::FirstAvailableScheduler>();
    scheduler_->SetActorRegister(actor_register_.get());
    scheduler_->SetScheduleFunction(schedule_event);
    scheduler_->SetNowFunction(now);
    scheduler_->SetMonitoredActor(cloud_handle_);

    event_loop_->SetUpdateWorldCallback([this] {
        WORLD_LOG_INFO("Updating world...");
        server_scheduler_manager_->ScheduleAll();
        scheduler_->UpdateSchedule();
        WORLD_LOG_INFO("Updating world... ok");
    });

    server_ = std::make_unique<SimulatorRPCService>();
    server_->SetWorld(this);

    config_->ParseResources(cloud_handle_, actor_register_.get(),
                            server_scheduler_manager_.get());
}

void
sim::core::World::Listen()
{
    std::string server_address("0.0.0.0:" + std::to_string(config_->GetPort()));

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(server_.get());
    // Finally assemble the server.
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cerr << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();

    WORLD_LOG_INFO("Quit!");
}

sim::UUID
sim::core::World::ResolveName(const std::string& name)
{
    UUID uuid;
    try {
        uuid = actor_register_->GetActorHandle(name);
        return uuid;
    } catch (const std::out_of_range& e) {
        throw std::invalid_argument(fmt::format("Name {} is not found", name));
    }
}

void
sim::core::World::DoResourceAction(const std::string& resource_name,
                                   sim::infra::ResourceEventType event_type)
{
    auto resource_uuid = ResolveName(resource_name);

    if (!resource_uuid) {
        throw std::invalid_argument(
            fmt::format("Unknown resource name: {}", resource_name));
    }

    auto event = events::MakeEvent<ResourceEvent>(resource_uuid,
                                                  event_loop_->Now(), nullptr);

    event->type = event_type;

    schedule_event(event, false);
}

void
sim::core::World::CreateVM(
    const std::string& vm_name, const std::string& vm_workload_model,
    const std::unordered_map<std::string, std::string>& params)
{
    UUID vm_uuid;
    auto vm = actor_register_->Make<VM>(vm_name);
    vm_uuid = vm->GetUUID();

    auto workload_model = custom::GetWorkloadModel(vm_workload_model);

    workload_model->Setup(params);
    vm->SetWorkloadModel(workload_model);
    vm->SetVMStorage(vm_storage_handle_);

    auto vmst_event = events::MakeEvent<VMStorageEvent>(
        vm_storage_handle_, event_loop_->Now(), nullptr);
    vmst_event->type = VMStorageEventType::kVMCreated;
    vmst_event->vm_uuid = vm_uuid;

    schedule_event(vmst_event, false);
}

void
sim::core::World::SimulateAll()
{
    event_loop_->SimulateAll();
}

void
sim::core::World::DoProvisionVM(const std::string& vm_name)
{
    auto vm_uuid = ResolveName(vm_name);

    auto vmst_event = events::MakeEvent<VMStorageEvent>(
        vm_storage_handle_, event_loop_->Now(), nullptr);
    vmst_event->type = VMStorageEventType::kVMProvisionRequested;
    vmst_event->vm_uuid = vm_uuid;
    schedule_event(vmst_event, true);
}

void
sim::core::World::DoRebootVM(const std::string& vm_name)
{
    auto vm_uuid = ResolveName(vm_name);
}

void
sim::core::World::DoStopVM(const std::string& vm_name)
{
    auto vm_uuid = ResolveName(vm_name);

    auto stop_vm_event =
        events::MakeEvent<VMEvent>(vm_uuid, event_loop_->Now(), nullptr);
    stop_vm_event->type = VMEventType::kStop;

    schedule_event(stop_vm_event, false);
}

void
sim::core::World::DoDeleteVM(const std::string& vm_name)
{
    auto vm_uuid = ResolveName(vm_name);

    auto delete_vm_event =
        events::MakeEvent<VMEvent>(vm_uuid, event_loop_->Now(), nullptr);
    delete_vm_event->type = VMEventType::kDelete;

    schedule_event(delete_vm_event, false);
}
