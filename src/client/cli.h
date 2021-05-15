#pragma once

#include "replxx.hxx"

namespace sim::client {

class CLI
{
 public:
    [[nodiscard]] bool Setup();

    /// Returns only if exit was requested
    void RunLoop();

    void SetProcessCallback(const std::function<void(const std::string&)>& cb)
    {
        process_callback = cb;
    }

 private:
    void BindKeys();

    replxx::Replxx rx_;

    std::function<void(const std::string&)> process_callback;

    // words to be completed
    std::vector<std::string> examples{"help",      "boot",         "shutdown",
                                      "create-vm", "provision-vm", "delete-vm",
                                      "stop-vm"};

    // the path to the history file
    std::string history_file{"./client_history.txt"};

    // set the repl prompt
    std::string prompt{"\x1b[1;32msim\x1b[0m> "};

    // highlight specific words
    // a regex string, and a color
    // the order matters, the last match will take precedence
    using cl = replxx::Replxx::Color;
    std::vector<std::pair<std::string, cl>> regex_color{
        // single chars
        {"\\`", cl::BRIGHTCYAN},
        {"\\'", cl::BRIGHTBLUE},
        {"\\\"", cl::BRIGHTBLUE},
        {"\\-", cl::BRIGHTBLUE},
        {"\\+", cl::BRIGHTBLUE},
        {"\\=", cl::BRIGHTBLUE},
        {"\\/", cl::BRIGHTBLUE},
        {"\\*", cl::BRIGHTBLUE},
        {"\\^", cl::BRIGHTBLUE},
        {"\\.", cl::BRIGHTMAGENTA},
        {"\\(", cl::BRIGHTMAGENTA},
        {"\\)", cl::BRIGHTMAGENTA},
        {"\\[", cl::BRIGHTMAGENTA},
        {"\\]", cl::BRIGHTMAGENTA},
        {"\\{", cl::BRIGHTMAGENTA},
        {"\\}", cl::BRIGHTMAGENTA},

        // color keywords
        {"delete-vm", cl::CYAN},
        {"shutdown", cl::RED},
        {"boot", cl::GREEN},
        {"provision-vm", cl::BLUE},
        {"stop-vm", cl::GRAY},
        {"create-vm", cl::YELLOW},

        // commands
        {"help", cl::BRIGHTMAGENTA},
        {"quit", cl::BRIGHTMAGENTA},

        // numbers
        {"[\\-|+]{0,1}[0-9]+", cl::YELLOW},            // integers
        {"[\\-|+]{0,1}[0-9]*\\.[0-9]+", cl::YELLOW},   // decimals
        {"[\\-|+]{0,1}[0-9]+e[\\-|+]{0,1}[0-9]+",
         cl::YELLOW},   // scientific notation

        // strings
        {"\".*?\"", cl::BRIGHTGREEN},   // double quotes
        {"\'.*?\'", cl::BRIGHTGREEN},   // single quotes
    };
};

}   // namespace sim::client
