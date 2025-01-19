#include <spire/core/heartbeater.hpp>

namespace spire {
Heartbeater::Heartbeater(
    const boost::asio::any_io_executor& executor,
    std::function<void()>&& on_check,
    std::function<void()>&& on_dead)
    : _timer {std::make_shared<Timer>(executor, BEAT_INTERVAL)},
    _on_check {std::move(on_check)}, _on_dead {std::move(on_dead)} {
    _timeout_registration = _timer->register_timeout_handler([this] {
        _on_check();

        if (_dead_beats >= MAX_DEAD_BEATS) {
            stop();
            _on_dead();
            return;
        }

        if (steady_clock::now() <= _last_beat + BEAT_INTERVAL) return;

        ++_dead_beats;
    });
}

void Heartbeater::start() {
    pulse();
    _timer->start();
}

void Heartbeater::stop() {
    _timer->stop();
}

void Heartbeater::pulse() {
    _last_beat = steady_clock::now();
    _dead_beats = 0;
}
}
