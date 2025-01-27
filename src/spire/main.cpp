#include <boost/mysql/src.hpp>

#include <boost/mysql.hpp>
#include <spdlog/spdlog.h>
#include <spire/core/settings.hpp>
#include <spire/net/server.hpp>

int main() {
    using namespace spire;

    Settings::init();

    boost::asio::thread_pool workers {std::thread::hardware_concurrency() - 1};
    boost::asio::signal_set signals {workers.get_executor(), SIGINT, SIGTERM};
    net::Server server {workers.get_executor()};

    boost::mysql::pool_params db_params {};
    db_params.server_address.emplace_host_and_port(Settings::db_host().data(), Settings::db_port());
    db_params.database = Settings::db_name();
    db_params.username = Settings::db_user();
    db_params.password = Settings::db_password();
    db_params.thread_safe = true;
    db_params.initial_size = std::thread::hardware_concurrency();
    db_params.multi_queries = true;

    boost::mysql::connection_pool db_pool {workers.get_executor(), std::move(db_params)};
    db_pool.async_run(boost::asio::detached);

#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
    spdlog::info("spdlog log level: {}", to_string_view(spdlog::get_level()));

    signals.async_wait([&workers, &server, &db_pool](boost::system::error_code, int) {
        server.stop();
        db_pool.cancel();
        workers.stop();
    });

    server.start();

    workers.attach();
    workers.join();

    return EXIT_SUCCESS;
}
