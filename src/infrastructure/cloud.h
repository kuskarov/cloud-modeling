#pragma once

#include "data-center.h"
#include "resource.h"
#include "types.h"

namespace sim::infra {

class Cloud : public IResource
{
 public:
    Cloud() : IResource("Cloud") {}

    // for scheduler
    const auto& GetDataCenters() const { return data_centers_; }

    UUID GetVMStorage() const { return vm_storage_; }

    void SetVMStorage(UUID vm_storage_handle)
    {
        vm_storage_ = vm_storage_handle;
    }

    void AddDataCenter(UUID uuid)
    {
        data_centers_.push_back(uuid);
        AddComponent(uuid);
    }

 private:
    std::vector<UUID> data_centers_{};
    UUID vm_storage_{};
};

}   // namespace sim::infra
