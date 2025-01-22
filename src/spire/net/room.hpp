#pragma once

#include <entt/entt.hpp>
#include <spire/net/client.hpp>
#include <spire/net/handler/message_handler.hpp>

#include <unordered_set>

namespace spire::net {
class Room final : std::enable_shared_from_this<Room>, boost::noncopyable {
public:
    Room(
        u32 id,
        boost::asio::strand<boost::asio::any_io_executor>&& strand,
        MessageHandler&& message_handler);

    void start();
    void stop();

    void add_client_deferred(std::shared_ptr<Client> client);
    void remove_client_deferred(std::shared_ptr<Client> client);

    void handle_message_deferred(std::unique_ptr<InMessage> message);
    void broadcast_message_deferred(std::shared_ptr<OutMessage> message);

    u32 id() const { return _id; }

private:
    void update(time_point<system_clock> last_update_time);

    const u32 _id;
    std::atomic<bool> _is_running {false};
    boost::asio::strand<boost::asio::any_io_executor> _strand;

    MessageHandler _message_handler;

    std::unordered_set<std::shared_ptr<Client>> _clients {};
    entt::registry _registry {};
};
}