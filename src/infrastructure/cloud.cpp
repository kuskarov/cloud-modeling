#include "cloud.h"

void
sim::infra::Cloud::StartBoot(const sim::infra::ResourceEvent* resource_event)
{
    LOG_F(INFO, "Cloud::StartBoot");

    for (const auto& data_center_ptr : data_centers_) {
        auto dc_boot_event = new ResourceEvent();
        dc_boot_event->resource_event_type = ResourceEventType::kBoot;
        dc_boot_event->addressee = data_center_ptr.get();
        dc_boot_event->happen_time = resource_event->happen_time;

        schedule_event(dc_boot_event, true);
    }

    Resource::StartBoot(resource_event);
}
