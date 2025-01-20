#pragma once

#include <entt/entt.hpp>
#include <spire/net/client.hpp>

#include <unordered_set>

namespace spire::net {
class Room final : boost::noncopyable {
public:
    Room(u32 id, boost::asio::strand<boost::asio::any_io_executor>&& strand);

    void start();
    void stop();

    void add_client_deferred(std::shared_ptr<Client> client);
    void remove_client_deferred(std::shared_ptr<Client> client);

    u32 id() const { return _id; }

private:
    void update(time_point<system_clock> last_update_time);

    const u32 _id;
    std::atomic<bool> _is_running {false};
    boost::asio::strand<boost::asio::any_io_executor> _strand;

    std::unordered_set<std::shared_ptr<Client>> _clients {};
    entt::registry _registry {};
};
}