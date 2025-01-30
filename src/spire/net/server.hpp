#pragma once

#include <spire/net/client.hpp>
#include <spire/net/room.hpp>
#include <taskflow/taskflow.hpp>

namespace spire::net {
class Server final : boost::noncopyable {
public:
    explicit Server(boost::asio::any_io_executor&& io_executor);
    ~Server();

    void start();
    void stop();

    void add_client_deferred(boost::asio::ip::tcp::socket&& socket);
    void remove_client_deferred(std::shared_ptr<Client> client);
    void transfer_client_deferred(std::shared_ptr<Client> client, u32 target_room_id);

    void add_room_deferred(std::shared_ptr<Room> room);
    void remove_room_deferred(u32 room_id);

private:
    std::atomic<bool> _is_running {false};

    boost::asio::any_io_executor _io_executor;
    boost::asio::strand<boost::asio::any_io_executor> _io_strand;
    tf::Executor _work_executor {};

    boost::asio::ip::tcp::acceptor _acceptor;

    std::unordered_map<u64, std::shared_ptr<Client>> _clients {};
    std::shared_ptr<Room> _waiting_room;
    std::shared_ptr<Room> _admin_room;
    std::unordered_map<u32, std::shared_ptr<Room>> _rooms {};
};
}