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

typedef std::function<infra::IWorkloadModel*()> WorkloadModelCreator;

template <typename WorkloadModel>
static WorkloadModelCreator
MakeWorkloadModel()
{
    static_assert(std::is_base_of_v<infra::IWorkloadModel, WorkloadModel>);

    return []() -> infra::IWorkloadModel* { return new WorkloadModel{}; };
}

}   // namespace sim::custom
