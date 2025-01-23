#include <spire/net/client.hpp>
#include <spire/net/room.hpp>
#include <spire/msg/base_message.pb.h>

namespace spire::net {
Client::Client(
    boost::asio::ip::tcp::socket&& socket,
    std::function<void(std::shared_ptr<Client>)>&& on_stop,
    const std::shared_ptr<Room>& current_room)
    : _strand {make_strand(socket.get_executor())},
    _heartbeater {
          socket.get_executor(),
          [self = shared_from_this()] {
              msg::Heartbeat heartbeat;
              msg::BaseMessage base;
              base.set_allocated_heartbeat(&heartbeat);

              self->send(std::make_unique<OutMessage>(&base));
          },
          [self = shared_from_this()] {
              self->stop(StopCode::DeadHeartbeat);
          }},
    _connection {
        std::move(socket),
        [self = shared_from_this()](const Connection::CloseCode code) {
            self->stop(code == Connection::CloseCode::Normal ? StopCode::Normal : StopCode::ConnectionError);
        },
        [self = shared_from_this()](std::vector<std::byte>&& data) {
            if (self->_is_authenticated) {
                self->_heartbeater.pulse();
            }

            if (const auto room {self->_current_room.load().lock()}; room) {
                room->handle_message_deferred(
                    std::make_unique<InMessage>(self->shared_from_this(), std::move(data)));
            }
        }},
    _on_stop {std::move(on_stop)},
    _current_room {current_room} {}

void Client::start() {
    if (_is_running.exchange(true)) return;

    _connection.open();
}

void Client::stop(const StopCode code) {
    if (!_is_running.exchange(false)) return;

    if (code != StopCode::Normal) {
        //TODO: Log
    }

    leave_room_deferred();

    _connection.close(Connection::CloseCode::Normal);

    _on_stop(shared_from_this());
}

void Client::send(std::unique_ptr<OutMessage> message) {
    _connection.send(std::move(message));
}

void Client::send(std::shared_ptr<OutMessage> message) {
    _connection.send(std::move(message));
}

void Client::authenticate() {
    _is_authenticated = true;
}

void Client::enter_room_deferred(std::shared_ptr<Room> room) {
    dispatch(_strand, [self = shared_from_this(), room = std::move(room)] mutable {
        if (const auto previous_room {self->_current_room.load().lock()}) {
            previous_room->remove_client_deferred(self);
        }

        self->_current_room = room;
        room->add_client_deferred(std::move(self));
    });
}

void Client::leave_room_deferred() {
    dispatch(_strand, [self = shared_from_this()] mutable {
        const auto previous_room {self->_current_room.load().lock()};
        if (!previous_room) return;

        self->_current_room.load().reset();
        previous_room->remove_client_deferred(std::move(self));
    });
}
}
