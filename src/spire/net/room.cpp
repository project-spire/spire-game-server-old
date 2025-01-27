#include <spire/net/room.hpp>
#include <spire/system/physics_system.hpp>

namespace spire::net {
Room::Room(
    const u32 id,
    boost::asio::any_io_executor& io_executor,
    tf::Executor& system_executor,
    MessageHandler&& message_handler)
    : _id {id},
    _io_executor {io_executor},
    _system_executor {system_executor},
    _message_handler {std::move(message_handler)} {}

Room::~Room() {
    stop();
}

void Room::start() {
    if (_is_running.exchange(true)) return;

    post(_io_executor, [self = shared_from_this()] {
        self->update(steady_clock::now());
    });
}

void Room::stop() {
    if (!_is_running.exchange(false)) return;

    //TODO: Cleanup <- Data race with update function
}

void Room::add_client_deferred(std::shared_ptr<Client> client) {
    _tasks.push([this, client = std::move(client)] mutable {
        _clients.insert_or_assign(client->id(), client);

        on_client_entered(client);
    });
}

void Room::remove_client_deferred(std::shared_ptr<Client> client) {
    _tasks.push([this, client = std::move(client)] {
        _clients.erase(client->id());

        on_client_left(client);
    });
}

void Room::post_task(std::function<void()>&& task) {
    _tasks.push(std::move(task));
}

void Room::post_message(std::unique_ptr<InMessage> message) {
    _messages.push(std::move(message));
}

void Room::broadcast_message_deferred(std::shared_ptr<OutMessage> message) {
    _tasks.push([self = shared_from_this(), message = std::move(message)] {
        for (const auto& client : self->_clients | std::views::values)
            client->send(message);
    });
}

void Room::update(const time_point<steady_clock> last_update_time) {
    if (!_is_running) return;

    const auto now {steady_clock::now()};
    const f32 dt {duration<f32, std::milli> {now - last_update_time}.count()};

    std::queue<std::unique_ptr<InMessage>> messages;
    _messages.swap(messages);
    while (!messages.empty()) {
        _message_handler.handle_message(std::move(messages.front()));
        messages.pop();
    }

    std::queue<std::function<void()>> tasks;
    _tasks.swap(tasks);
    while (!tasks.empty()) {
        tasks.front()();
        tasks.pop();
    }

    tf::Taskflow system_taskflow {};
    compose_systems(system_taskflow, now, dt);

    auto update_more = [self = shared_from_this(), now] mutable {
        post(self->_io_executor, [self = std::move(self), now] {
            self->update(now);
        });
    };

    _system_executor.run(std::move(system_taskflow), std::move(update_more));
}
}
