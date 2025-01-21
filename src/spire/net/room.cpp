#include <spire/net/room.hpp>
#include <spire/system/physics_system.hpp>

namespace spire::net {
Room::Room(const u32 id, boost::asio::strand<boost::asio::any_io_executor>&& strand)
    : _id {id}, _strand {std::move(strand)} {}

void Room::start() {
    if (_is_running.exchange(true)) return;

    post(_strand, [self = shared_from_this()] {
        self->update(high_resolution_clock::now());
    });
}

void Room::stop() {
    if (!_is_running.exchange(false)) return;
}

void Room::add_client_deferred(std::shared_ptr<Client> client) {
    post(_strand, [self = shared_from_this(), client = std::move(client)] mutable {
        self->_clients.insert(std::move(client));
    });
}

void Room::remove_client_deferred(std::shared_ptr<Client> client) {
    post(_strand, [self = shared_from_this(), client = std::move(client)] {
        self->_clients.erase(client);
    });
}

void Room::handle_message_deferred(std::unique_ptr<InMessage> message) {
    dispatch(_strand, [self = shared_from_this(), message = std::move(message)] mutable {
        //TODO;
    });
}

void Room::broadcast_message_deferred(std::shared_ptr<OutMessage> message) {
    dispatch(_strand, [self = shared_from_this(), message = std::move(message)] {
        for (const auto& client : self->_clients) {
            client->send(message);
        }
    });
}

void Room::update(const time_point<system_clock> last_update_time) {
    if (!_is_running) return;

    const auto now {high_resolution_clock::now()};
    const f32 dt {duration_cast<duration<f32, milliseconds>>(now - last_update_time).count()};

    physics::PhysicsSystem::update(_registry, dt);

    post(_strand, [self = shared_from_this(), now] {
        self->update(now);
    });
}
}
