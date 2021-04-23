#include "resource-register.h"

#include "resource.h"

std::unordered_map<std::string, sim::infra::IResource*>
    sim::infra::ResourceRegister::mapping_{};

void
sim::infra::ResourceRegister::RegisterResource(infra::IResource* resource)
{
    if (mapping_.count(resource->GetName())) {
        SimulatorLogger().PrintErrorToConsole("Name {} is already in use",
                                              resource->GetName());
    } else {
        mapping_[resource->GetName()] = resource;
        SimulatorLogger().PrintErrorToConsole("Registered resource {} :: {}",
                                              resource->GetType(),
                                              resource->GetName());
    }
}

sim::infra::IResource*
sim::infra::ResourceRegister::GetResourceHandle(const std::string& name)
{
    return mapping_.at(name);
}
