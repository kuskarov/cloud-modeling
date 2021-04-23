#include "cloud.h"

void
sim::infra::Cloud::StartBoot(const ResourceEvent* resource_event)
{
    for (const auto& data_center_ptr : data_centers_) {
        auto dc_boot_event = new ResourceEvent();
        dc_boot_event->type = ResourceEventType::kBoot;
        dc_boot_event->addressee = data_center_ptr.get();
        dc_boot_event->happen_time = resource_event->happen_time;

        schedule_event(dc_boot_event, true);
    }

    IResource::StartBoot(resource_event);
}

void
sim::infra::Cloud::StartShutdown(const ResourceEvent* resource_event)
{
    for (const auto& data_center_ptr : data_centers_) {
        auto dc_shutdown_event = new ResourceEvent();
        dc_shutdown_event->type = ResourceEventType::kShutdown;
        dc_shutdown_event->addressee = data_center_ptr.get();
        dc_shutdown_event->happen_time = resource_event->happen_time;

        schedule_event(dc_shutdown_event, false);
    }

    IResource::StartShutdown(resource_event);
}
