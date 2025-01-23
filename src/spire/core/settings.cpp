#include <spire/core/settings.hpp>

#include <cstdlib>
#include <fstream>
#include <stdexcept>

namespace spire {
std::string read_file_line(std::string_view path) {
    std::ifstream file {path.data()};
    if (!file.is_open())
        throw std::invalid_argument("Invalid file path");

    std::string value;
    std::getline(file, value);
    return value;
}

// TODO
// yaml parse_yaml_file(std::string_view path) {}

void Settings::init() {
    // _listen_port = std::stoi(std::getenv("SPIRE_GAME_LISTEN_PORT"));
    _listen_port = 30000;
    _listen_backlog = 4096;
}
}
