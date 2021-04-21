#include "logger.h"

#include "types.h"

std::unique_ptr<spdlog::logger> sim::SimulatorLogger::logger_{};
std::string sim::SimulatorLogger::csv_file_name_{};
std::function<sim::types::TimeStamp()> sim::SimulatorLogger::now{};
