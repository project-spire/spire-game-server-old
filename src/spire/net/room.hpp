#pragma once

#include <entt/entt.hpp>
#include <spire/container/concurrent_queue.hpp>
#include <spire/net/client.hpp>
#include <spire/net/message_handler.hpp>
#include <taskflow/taskflow.hpp>

namespace spire::net {
class Room final : std::enable_shared_from_this<Room>, boost::noncopyable {
public:
    Room(
        u32 id,
        boost::asio::any_io_executor& io_executor,
        tf::Executor& system_executor,
        MessageHandler&& message_handler);

    void start();
    void stop();

    void add_client_deferred(std::shared_ptr<Client> client);
    void remove_client_deferred(std::shared_ptr<Client> client);

    void post_task(std::function<void()>&& task);
    void post_message(std::unique_ptr<InMessage> message);
    void broadcast_message_deferred(std::shared_ptr<OutMessage> message);

    u32 id() const { return _id; }

private:
    void update(time_point<steady_clock> last_update_time);

    const u32 _id;

    std::atomic<bool> _is_running {false};
    boost::asio::any_io_executor& _io_executor;
    tf::Executor& _system_executor;

    ConcurrentQueue<std::function<void()>> _tasks {};
    ConcurrentQueue<std::unique_ptr<InMessage>> _messages {};
    MessageHandler _message_handler;

    std::unordered_map<u64, std::shared_ptr<Client>> _clients {};
    entt::registry _registry {};
};
}