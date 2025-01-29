#include <spire/core/settings.hpp>
#include <spire/net/heartbeat.hpp>

namespace spire::net {
Heartbeat::Heartbeat(
    const boost::asio::any_io_executor& executor,
    std::function<void()>&& on_retry,
    std::function<void()>&& on_dead)
    : _timer {executor, Settings::heartbeat_interval()},
    _on_retry {std::move(on_retry)},
    _on_dead {std::move(on_dead)} {
    _timer.add_timeout_callback([this] {
        if (_retries >= Settings::heartbeat_retries()) {
            stop();
            _on_dead();
            return;
        }

        if (steady_clock::now() <= _last_retry + Settings::heartbeat_interval()) return;

        ++_retries;
        _on_retry();
    });
}

void Heartbeat::start() {
    reset();
    _timer.start();
}

void Heartbeat::stop() {
    _timer.stop();
}

void Heartbeat::reset() {
    _last_retry = steady_clock::now();
    _retries = 0;
}
}
