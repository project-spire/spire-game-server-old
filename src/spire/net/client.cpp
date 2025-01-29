#include <spdlog/spdlog.h>
#include <spire/net/client.hpp>
#include <spire/net/room.hpp>
#include <spire/msg/base_message.pb.h>

namespace spire::net {
Client::Client(
    const u64 id,
    boost::asio::ip::tcp::socket&& socket,
    const std::shared_ptr<Room>& current_room,
    std::function<void(std::shared_ptr<Client>)>&& on_stop)
    : _strand {make_strand(socket.get_executor())},
    _heartbeat {
          socket.get_executor(),
          [this] {
              msg::BaseMessage base;
              base.set_allocated_heartbeat(new msg::Heartbeat);

              send(std::make_unique<OutMessage>(base));
          },
          [this] {
              stop(StopCode::HeartbeatDead);
          }},
    _connection {std::move(socket)},
    _on_stop {std::move(on_stop)},
    _current_room {current_room},
    _character_id {id} {}

Client::~Client() {
    stop(StopCode::Normal);
}

std::shared_ptr<Client> Client::make(
    const u64 id,
    boost::asio::ip::tcp::socket&& socket,
    const std::shared_ptr<Room>& current_room,
    std::function<void(std::shared_ptr<Client>)>&& on_stop) {
    auto client {std::make_shared<Client>(id, std::move(socket), current_room, std::move(on_stop))};

    client->_connection.init(
        [self = client->shared_from_this()](const Connection::CloseCode code) {
            self->stop(code == Connection::CloseCode::Normal ? StopCode::Normal : StopCode::ConnectionError);
        },
        [self = client->shared_from_this()](std::vector<std::byte>&& data) {
            if (self->_is_authenticated) {
                self->_heartbeat.reset();
            }

            const auto room {self->_current_room.load().lock()};
            if (!room) return;
            room->post_message(std::make_unique<InMessage>(self->shared_from_this(), std::move(data)));
        });

    return client;
}

void Client::start() {
    if (_is_running.exchange(true)) return;

    _connection.open();
    _heartbeat.start();
}

void Client::stop(const StopCode code) {
    if (!_is_running.exchange(false)) return;

    if (code != StopCode::Normal) {
        spdlog::debug("{} stopped abnormally with code {}", display(), std::to_underlying(code));
    }

    _connection.close(Connection::CloseCode::Normal);
    _heartbeat.stop();

    _on_stop(shared_from_this());
}

void Client::send(std::unique_ptr<OutMessage> message) {
    _connection.send(std::move(message));
}

void Client::send(std::shared_ptr<OutMessage> message) {
    _connection.send(std::move(message));
}

void Client::authenticate(const u32 account_id, const u32 character_id) {
    _is_authenticated = true;

    _account_id = account_id;
    _character_id = character_id;
}

std::string Client::display() const {
    return std::format("Client {{ account_id: {}, character_id: {} }}"sv,
        _account_id, _character_id);
}
}
