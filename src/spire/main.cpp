#include <spdlog/spdlog.h>
#include <spire/core/settings.hpp>
#include <spire/server/server.hpp>

int main() {
    using namespace spire;

    Settings::init();

#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
    spdlog::info("spdlog log level: {}", to_string_view(spdlog::get_level()));

    boost::asio::thread_pool io_threads {std::thread::hardware_concurrency() - 1};
    boost::asio::signal_set signals {io_threads.get_executor(), SIGINT, SIGTERM};
    Server server {io_threads.get_executor()};

    signals.async_wait([&](boost::system::error_code, int) {
        server.stop();
        io_threads.stop();
    });

    server.start();

    io_threads.attach();
    io_threads.join();

    return EXIT_SUCCESS;
}
