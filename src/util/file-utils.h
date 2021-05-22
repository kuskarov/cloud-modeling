#include <filesystem>
#include <string>

static auto
FileExists(std::string_view path) -> bool
{
    return std::filesystem::exists(path);
}
