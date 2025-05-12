#include "shadow_handler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// Helper function to read file contents into a string
std::string ReadFileContents(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + file_path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        ShadowExample::PrintHelp();
        return 1;
    }

    Aws::Crt::String thing_name = "exampleThing";
    ShadowExample handler(thing_name);

    if (handler.Initialize() != sv::Status::kSuccess) {
        std::cerr << "Failed to initialize ShadowExample\n";
        return 1;
    }

    std::string command = argv[1];
    if (command == "get") {
        if (argc != 3) {
            std::cerr << "Error: 'get' command requires a shadow name.\n";
            ShadowExample::PrintHelp();
            return 1;
        }
        Aws::Crt::String shadow_name = argv[2];
        handler.GetThingShadow(shadow_name);
    } else if (command == "update") {
        if (argc != 4) {
            std::cerr << "Error: 'update' command requires a shadow name and a file path for the payload.\n";
            ShadowExample::PrintHelp();
            return 1;
        }
        Aws::Crt::String shadow_name = argv[2];
        std::string file_path = argv[3];
        try {
            std::string payload = ReadFileContents(file_path);
            handler.UpdateThingShadow(shadow_name, payload);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    } else if (command == "list") {
        handler.ListNamedShadowsForThing();
    } else if (command == "help") {
        ShadowExample::PrintHelp();
    } else {
        std::cerr << "Invalid command or arguments\n";
        ShadowExample::PrintHelp();
        return 1;
    }

    return 0;
}
