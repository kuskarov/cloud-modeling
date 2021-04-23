#include "data-center.h"

#include "logger.h"

void
sim::infra::DataCenter::StartBoot(const ResourceEvent* resource_event)
{
    for (const auto& server : servers_) {
        auto server_boot_event = new ResourceEvent();
        server_boot_event->type = ResourceEventType::kBoot;
        server_boot_event->addressee = server.get();
        server_boot_event->happen_time = resource_event->happen_time;

        schedule_event(server_boot_event, false);
    }

    IResource::StartBoot(resource_event);
}

void
sim::infra::DataCenter::StartShutdown(const ResourceEvent* resource_event)
{
    for (const auto& server : servers_) {
        auto server_shutdown_event = new ResourceEvent();
        server_shutdown_event->type = ResourceEventType::kShutdown;
        server_shutdown_event->addressee = server.get();
        server_shutdown_event->happen_time = resource_event->happen_time;

        schedule_event(server_shutdown_event, false);
    }

    IResource::StartShutdown(resource_event);
}
