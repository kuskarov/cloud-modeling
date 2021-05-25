#pragma once

#include <unordered_map>

// Cloud schedulers
#include "cloud-schedulers/place-to-first.h"

// Server schedulers
#include "server-schedulers/greedy.h"

// Base classes
#include "scheduler.h"
#include "vm.h"

// Other
#include "util.h"

namespace sim::custom {

static auto
GetScheduler(const std::string& name)
{
    static std::unordered_map<std::string, SchedulerCreator> mapping = {
        {"greedy", MakeScheduler<FirstAvailableScheduler>()}};

    return mapping.at(name)();
}

static auto
GetWorkloadModel(const std::string& name)
{
    static std::unordered_map<std::string, WorkloadModelCreator> mapping = {
        {"constant", MakeWorkloadModel<ConstantVMWorkloadModel>()}};

    return mapping.at(name)();
}

}   // namespace sim::custom
