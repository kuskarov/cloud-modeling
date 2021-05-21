#include <filesystem>
#include <string>

static auto
FileExists(const std::string& path) -> bool
{
    return std::filesystem::exists(path);
}
