#pragma once

namespace sim::custom {

typedef std::function<IScheduler*()> SchedulerCreator;

template <typename Scheduler>
static SchedulerCreator
MakeScheduler()
{
    static_assert(std::is_base_of_v<IScheduler, Scheduler>);

    return []() -> IScheduler* { return new Scheduler{}; };
}

typedef std::function<infra::IVMWorkloadModel*()> WorkloadModelCreator;

template <typename WorkloadModel>
static WorkloadModelCreator
MakeWorkloadModel()
{
    static_assert(std::is_base_of_v<infra::IVMWorkloadModel, WorkloadModel>);

    return []() -> infra::IVMWorkloadModel* { return new WorkloadModel{}; };
}

}   // namespace sim::custom
