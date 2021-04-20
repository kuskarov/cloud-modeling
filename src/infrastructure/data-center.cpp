#include "data-center.h"

#include <loguru.hpp>

void
sim::infra::DataCenter::StartBoot(const ResourceEvent* resource_event)
{
    LOG_F(INFO, "DataCenter::StartBoot");

    for (const auto& server : servers_) {
        auto server_boot_event = std::make_shared<ResourceEvent>();
        server_boot_event->resource_event_type = ResourceEventType::kBoot;
        server_boot_event->addressee = server.get();
        server_boot_event->happen_ts = resource_event->happen_ts;

        schedule_callback_(resource_event->happen_ts, server_boot_event, false);
    }

    Resource::StartBoot(resource_event);
}
