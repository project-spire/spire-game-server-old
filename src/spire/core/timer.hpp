#pragma once

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include <chrono>

namespace spire {
using namespace std::chrono;

class Timer final {
public:
    Timer(
        const boost::asio::any_io_executor& executor,
        milliseconds duration,
        bool one_shot = false);
    ~Timer();

    void start();
    void stop();

    boost::signals2::connection add_timeout_callback(std::function<void()>&& callback);

private:
    const milliseconds _duration;
    const bool _one_shot;

    boost::asio::steady_timer _timer;
    boost::signals2::signal<void()> _timeout {};

    const boost::asio::any_io_executor& _executor;
    boost::asio::cancellation_signal _cancelled {};
    std::atomic<bool> _is_running {false};
};
}
