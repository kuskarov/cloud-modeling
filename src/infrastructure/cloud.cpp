#include "cloud.h"

void
sim::infra::Cloud::HandleEvent(const std::shared_ptr<events::Event>& event)
{
    Resource::HandleEvent(event);
}
