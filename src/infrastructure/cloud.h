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
    [[nodiscard]] const auto& DataCenters() const { return data_centers_; }

    [[nodiscard]] auto VMStorage() const { return vm_storage_; }

    void SetVMStorage(types::UUID vm_storage_handle)
    {
        vm_storage_ = vm_storage_handle;
    }

    types::EnergyCount SpentPower() override { return types::EnergyCount{0}; }

    void AddDataCenter(types::UUID uuid)
    {
        data_centers_.push_back(uuid);
        AddComponent(uuid);
    }

 private:
    std::vector<types::UUID> data_centers_{};
    types::UUID vm_storage_{};
};

}   // namespace sim::infra
