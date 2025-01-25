#include <spdlog/spdlog.h>
#include <spire/core/settings.hpp>
#include <spire/net/server.hpp>

int main() {
    boost::asio::thread_pool workers {std::thread::hardware_concurrency() - 1};
    boost::asio::signal_set signals {workers.get_executor(), SIGINT, SIGTERM};
    spire::net::Server server {workers.get_executor()};

    spire::Settings::init();

#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
    spdlog::info("spdlog log level: {}", to_string_view(spdlog::get_level()));

    signals.async_wait([&workers, &server](boost::system::error_code, int) {
        server.stop();
        workers.stop();
    });

    server.start();

    workers.attach();
    workers.join();

    return EXIT_SUCCESS;
}
