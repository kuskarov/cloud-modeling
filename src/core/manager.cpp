#include "manager.h"

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
sim::core::Manager::Setup()
{
    SimulatorLogger::SetCSVFolder(config_->GetLogsPath());
    SimulatorLogger::SetMaxCSVSeverity(LogSeverity::kDebug);
    SimulatorLogger::SetMaxConsoleSeverity(LogSeverity::kDebug);
    SimulatorLogger::SetTimeCallback([this] { return event_loop_->Now(); });

    event_loop_ = std::make_unique<events::EventLoop>();

    schedule_event = [this](events::Event* event, bool immediate) {
        event_loop_->Insert(event, immediate);
    };

    actor_register_ = std::make_unique<ActorRegister>();
    actor_register_->SetScheduleFunction(schedule_event);

    server_scheduler_manager_ = std::make_unique<ServerSchedulerManager>();
    server_scheduler_manager_->SetScheduleFunction(schedule_event);
    server_scheduler_manager_->SetActorRegister(actor_register_.get());

    event_loop_->SetActorFromUUIDCallback(
        [this](types::UUID uuid) -> events::IActor* {
            return actor_register_->GetActor<events::IActor>(uuid);
        });

    auto cloud = actor_register_->Make<infra::Cloud>("cloud-1");
    cloud_handle_ = cloud->UUID();

    auto vm_storage = actor_register_->Make<infra::VMStorage>("vm-storage-1");
    vm_storage_handle_ = vm_storage->UUID();

    cloud->SetVMStorage(vm_storage_handle_);

    auto scheduler =
        actor_register_->Make<custom::FirstAvailableScheduler>("scheduler-1");
    scheduler->SetActorRegister(actor_register_.get());
    scheduler->SetCloud(cloud_handle_);

    scheduler_handle_ = scheduler->UUID();

    event_loop_->SetUpdateWorldCallback([this] {
        WORLD_LOG_INFO("Updating world...");
        server_scheduler_manager_->ScheduleAll();
        WORLD_LOG_INFO("Updating world... ok");
    });

    server_ = std::make_unique<SimulatorRPCService>();
    server_->SetScheduleFunction(schedule_event);
    server_->SetActorRegister(actor_register_.get());
    server_->SetEventLoop(event_loop_.get());
    server_->SetHandles(cloud_handle_, scheduler_handle_, vm_storage_handle_);

    config_->ParseResources(cloud_handle_, actor_register_.get(),
                            server_scheduler_manager_.get());
}

void
sim::core::Manager::Listen()
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
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();

    WORLD_LOG_INFO("Quit!");
}
