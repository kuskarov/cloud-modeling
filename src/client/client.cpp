#include <argparse.hpp>
#include <iostream>

#include "cli.h"
#include "rpc-client.h"

int
main(int argc, char** argv)
{
    argparse::ArgumentParser parser("client");

    parser.add_argument("--host")
        .help("Host of the simulator engine")
        .nargs(1)
        .required();

    parser.add_argument("--port")
        .help("Port of the simulator engine")
        .nargs(1)
        .required();

    std::string host_port;
    try {
        parser.parse_args(argc, argv);

        host_port = parser.get<std::string>("--host") + ":" +
                    parser.get<std::string>("--port");

    } catch (const std::runtime_error& re) {
        std::cerr << "Arguments parse error: " << re.what() << "\n";
        std::cerr << parser;
        return 1;
    }

    sim::client::SimulatorRPCClient client{sim::client::CreateChannel(host_port)};

    if (!client.Setup()) {
        std::cerr << "RPC-Client setup failed!\n";
        return 1;
    }

    sim::client::CLI cli{};

    cli.SetProcessCallback(
        [&client](const std::string& input) { client.ProcessInput(input); });

    if (!cli.Setup()) {
        std::cerr << "CLI setup failed!\n";
        return 1;
    }

    // display initial welcome message
    std::cout << "Welcome to the Cloud Simulator\n"
              << "Type 'help' for help\n"
              << "Type 'quit' to exit\n\n";

    cli.RunLoop();

    std::cout << "\nExiting Simulator\n";
}
