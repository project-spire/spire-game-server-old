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

void Room::start() {
    if (_is_running.exchange(true)) return;

    post(_io_executor, [self = shared_from_this()] {
        self->update(steady_clock::now());
    });
}

void Room::stop() {
    if (!_is_running.exchange(false)) return;
}

void Room::add_client_deferred(std::shared_ptr<Client> client) {
    _tasks.push([self = shared_from_this(), client = std::move(client)] mutable {
        self->_clients.insert_or_assign(client->id(), std::move(client));
    });
}

void Room::remove_client_deferred(std::shared_ptr<Client> client) {
    _tasks.push([self = shared_from_this(), client = std::move(client)] {
        self->_clients.erase(client->id());
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

    // TODO: Make the taskflow once, and reuse it.
    tf::Taskflow system_taskflow {};

    auto [physics_task, my_task] = system_taskflow.emplace(
        [this, dt] { physics::PhysicsSystem::update(_registry, dt); },
        [this, dt] {/*Do another task*/}
    );
    physics_task.precede(my_task);


    auto update_more = [self = shared_from_this(), now] mutable {
        post(self->_io_executor, [self = std::move(self), now] {
            self->update(now);
        });
    };

    _system_executor.run(system_taskflow, std::move(update_more));
}
}
