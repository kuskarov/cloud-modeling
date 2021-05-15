#include <filesystem>
#include <string>

static inline auto
FileExists(const std::string& path) -> bool
{
    return std::filesystem::exists(path);
}
