#pragma once

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include <chrono>

namespace spire {
using namespace std::chrono;

class Timer final : std::enable_shared_from_this<Timer> {
public:
    Timer(
        boost::asio::any_io_executor& executor, milliseconds duration,
        bool auto_start = false, bool one_shot = false);
    ~Timer();

    void start();
    void stop();

    boost::signals2::connection register_timeout_handler(std::function<void()>&& handler);

private:
    const milliseconds _duration;
    const bool _one_shot;
    boost::asio::steady_timer _timer;
    boost::signals2::signal<void()> _on_timeout {};

    boost::asio::any_io_executor& _executor;
    std::atomic<bool> _is_running {false};
};
}
