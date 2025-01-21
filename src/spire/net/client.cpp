#include <spire/net/client.hpp>
#include <spire/net/room.hpp>
#include <spire/msg/base_message.pb.h>

namespace spire::net {
Client::Client(
    boost::asio::ip::tcp::socket&& socket,
    std::function<void(std::shared_ptr<Client>)>&& on_stop)
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
              self->stop();
          }},
    _connection {
        std::move(socket),
        [self = shared_from_this()](const Connection::CloseCode) {
            self->stop();
        },
        [self = shared_from_this()](std::vector<std::byte>&& data) {
            if (_is_authenticated) {
                _heartbeater.pulse();
            }

            auto message {std::make_unique<InMessage>(self->shared_from_this(), std::move(data))};

            dispatch(self->_strand, [message = std::move(message)] mutable {
                const auto current_room {message->client()->_current_room.lock()};
                if (!current_room) return;
                current_room->handle_message_deferred(std::move(message));
            });
        }},
    _on_stop {std::move(on_stop)} {}

void Client::start() {
    if (_is_running.exchange(true)) return;

    _connection.open();
}

void Client::stop() {
    if (!_is_running.exchange(false)) return;

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
    post(_strand, [self = shared_from_this(), room = std::move(room)] mutable {
        if (const auto previous_room {self->_current_room.lock()}) {
            previous_room->remove_client_deferred(self);
        }

        self->_current_room = room;
        room->add_client_deferred(std::move(self));
    });
}

void Client::leave_room_deferred() {
    post(_strand, [self = shared_from_this()] mutable {
        const auto previous_room {self->_current_room.lock()};
        if (!previous_room) return;

        previous_room->remove_client_deferred(std::move(self));
        self->_current_room.reset();
    });
}
}
