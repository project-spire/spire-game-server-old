#include <spire/core/timer.hpp>

namespace spire {
Timer::Timer(const boost::asio::any_io_executor& executor, const milliseconds duration,
    const bool auto_start, const bool one_shot)
    : _duration {duration}, _one_shot {one_shot}, _timer {executor}, _executor {executor} {
    if (auto_start) start();
}

Timer::~Timer() {
    stop();
}

void Timer::start() {
    if (_is_running.exchange(true)) return;

    // Capture "self" to ensure lifetime
    post(_executor, [self = shared_from_this()]()->boost::asio::awaitable<void> {
        do {
            self->_timer.expires_after(self->_duration);

            if (auto [ec] = co_await self->_timer.async_wait(boost::asio::as_tuple(boost::asio::use_awaitable));
                ec || !self->_is_running) {
                co_return;
            }

            self->_on_timeout();
        } while (!self->_one_shot && self->_is_running);
    });
}

void Timer::stop() {
    if (!_is_running.exchange(false)) return;

    _timer.cancel();
}

boost::signals2::connection Timer::register_timeout_handler(std::function<void()>&& handler) {
    return _on_timeout.connect(handler);
}
}
