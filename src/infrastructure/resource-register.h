#pragma once

#include "logger.h"
#include "resource.h"

namespace sim::infra {

class IResource;

class ResourceRegister
{
 public:
    static void RegisterResource(infra::IResource* resource);

    static infra::IResource* GetResourceHandle(const std::string& name);

 private:
    static std::unordered_map<std::string, infra::IResource*> mapping_;
};

}   // namespace sim::infra
