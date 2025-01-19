#pragma once

#include <boost/core/noncopyable.hpp>
#include <spire/core/timer.hpp>
#include <spire/core/types.hpp>

namespace spire {
class HeartBeater final : boost::noncopyable {
    static constexpr uint32_t MAX_DEAD_BEATS = 3;
    static constexpr auto BEAT_INTERVAL = 5s;

public:
    HeartBeater(
        const boost::asio::any_io_executor& executor,
        std::function<void()>&& on_check,
        std::function<void()>&& on_dead);

    void start();
    void stop();

    void pulse();

private:
    steady_clock::time_point _last_beat {};
    u32 _dead_beats {0};
    std::shared_ptr<Timer> _timer;

    std::function<void()> _on_check;
    std::function<void()> _on_dead;
    boost::signals2::connection _timeout_registration;
};
}