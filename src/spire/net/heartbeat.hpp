#pragma once

#include <boost/core/noncopyable.hpp>
#include <spire/core/timer.hpp>
#include <spire/core/types.hpp>

namespace spire::net {
class Heartbeat final : boost::noncopyable {
public:
    Heartbeat(
        const boost::asio::any_io_executor& executor,
        std::function<void()>&& on_retry,
        std::function<void()>&& on_dead);

    void start();
    void stop();

    void reset();

private:
    Timer _timer;
    steady_clock::time_point _last_retry {};
    u32 _retries {0};

    std::function<void()> _on_retry;
    std::function<void()> _on_dead;
};
}