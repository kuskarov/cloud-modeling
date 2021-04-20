#include <sys/stat.h>
#include <unistd.h>

#include <string>

inline bool
FileExists(const std::string& name)
{
    struct stat buffer
    {
    };
    return !stat(name.c_str(), &buffer);
}
