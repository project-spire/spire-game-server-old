#include <spire/core/timer.hpp>

namespace spire {
Timer::Timer(
    const boost::asio::any_io_executor& executor,
    const milliseconds duration,
    const bool one_shot)
    : _duration {duration}, _one_shot {one_shot}, _timer {executor}, _executor {executor} {}

Timer::~Timer() {
    stop();
}

void Timer::start() {
    if (_is_running.exchange(true)) return;

    co_spawn(_executor, [this] -> boost::asio::awaitable<void> {
        do {
            _timer.expires_after(_duration);

            if (auto [ec] = co_await _timer.async_wait(boost::asio::as_tuple(boost::asio::use_awaitable));
                ec || !_is_running) {
                co_return;
            }

            _timeout();
        } while (!_one_shot && _is_running);
    }, bind_cancellation_slot(_cancelled.slot(), boost::asio::detached));
}

void Timer::stop() {
    if (!_is_running.exchange(false)) return;

    _cancelled.emit(boost::asio::cancellation_type::all);
    _timer.cancel();
}

boost::signals2::connection Timer::add_timeout_callback(std::function<void()>&& callback) {
    return _timeout.connect(callback);
}
}
