#pragma once

#include <spire/net/client.hpp>
#include <spire/net/room.hpp>

namespace spire::net {
class Server final : boost::noncopyable {
public:
    explicit Server(boost::asio::any_io_executor&& executor);
    ~Server();

    void start();
    void stop();

    void add_client_deferred(boost::asio::ip::tcp::socket&& socket);
    void remove_client_deferred(std::shared_ptr<Client> client);

private:
    std::atomic<bool> _is_running {false};
    boost::asio::any_io_executor _executor;
    boost::asio::strand<boost::asio::any_io_executor> _strand;
    boost::asio::ip::tcp::acceptor _acceptor;

    std::unordered_set<std::shared_ptr<Client>> _clients {};
    std::unordered_map<u32, std::shared_ptr<Room>> _rooms {};
    std::shared_ptr<Room> _waiting_room {};
};
}