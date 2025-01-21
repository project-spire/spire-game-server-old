#pragma once

#include <spire/net/connection.hpp>
#include <spire/net/heartbeater.hpp>
#include <spire/net/message.hpp>

namespace spire::net {
class Room;

class Client final : std::enable_shared_from_this<Client>, boost::noncopyable {
public:
    Client(
        boost::asio::ip::tcp::socket&& socket,
        std::function<void(std::shared_ptr<Client>)>&& on_stop);

    void start();
    void stop();

    void send(std::unique_ptr<OutMessage> message);
    void send(std::shared_ptr<OutMessage> message);

    void authenticate();

    void enter_room_deferred(std::shared_ptr<Room> room);
    void leave_room_deferred();

private:
    boost::asio::strand<boost::asio::any_io_executor> _strand;
    Heartbeater _heartbeater;
    Connection _connection;

    std::function<void(std::shared_ptr<Client>)> _on_stop;

    std::atomic<bool> _is_running {false};
    bool _is_authenticated {false};

    std::weak_ptr<Room> _current_room {};
};
}