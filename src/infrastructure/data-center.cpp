#include "data-center.h"

#include <loguru.hpp>

void
sim::infra::DataCenter::StartBoot(const ResourceEvent* resource_event)
{
    LOG_F(INFO, "DataCenter::StartBoot");

    for (const auto& server : servers_) {
        auto server_boot_event = new ResourceEvent();
        server_boot_event->resource_event_type = ResourceEventType::kBoot;
        server_boot_event->addressee = server.get();
        server_boot_event->happen_time = resource_event->happen_time;

        schedule_event(server_boot_event, false);
    }

    Resource::StartBoot(resource_event);
}
