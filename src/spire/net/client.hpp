#pragma once

#include <spire/net/connection.hpp>
#include <spire/net/heartbeater.hpp>
#include <spire/net/message.hpp>

namespace spire::net {
class Room;

class Client final : std::enable_shared_from_this<Client>, boost::noncopyable {
public:
    enum class StopCode {
        Normal,
        InvalidInMessage,
        ConnectionError,
        DeadHeartbeat,
    };

    Client(
        boost::asio::ip::tcp::socket&& socket,
        std::function<void(std::shared_ptr<Client>)>&& on_stop,
        const std::shared_ptr<Room>& current_room);

    void start();
    void stop(StopCode code);

    void send(std::unique_ptr<OutMessage> message);
    void send(std::shared_ptr<OutMessage> message);

    void authenticate(u32 character_id);

    void enter_room_deferred(std::shared_ptr<Room> room);
    void leave_room_deferred();

    u64 id() const { return _character_id; }
    milliseconds ping() const { return _ping.load(); }

private:
    boost::asio::strand<boost::asio::any_io_executor> _strand;
    Heartbeater _heartbeater;
    Connection _connection;
    std::atomic<milliseconds> _ping {};

    std::atomic<bool> _is_running {false};
    bool _is_authenticated {false};
    std::function<void(std::shared_ptr<Client>)> _on_stop;

    std::atomic<std::weak_ptr<Room>> _current_room;

    u64 _character_id {};
};
}