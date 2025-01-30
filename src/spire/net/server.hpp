#pragma once

#include <spire/net/client.hpp>
#include <spire/net/district.hpp>
#include <taskflow/taskflow.hpp>

namespace spire::net {
using GameClient = Client<TcpSocket>;
using GameDistrict = District<TcpRoom>;
using AdminClient = Client<SslClient>;

class Server final : boost::noncopyable {
public:
    explicit Server(boost::asio::any_io_executor&& io_executor);
    ~Server();

    void start();
    void stop();

    void add_game_client(TcpSocket&& socket);
    void add_admin_client(SslSocket&& socket);

    void remove_game_client_deferred(std::shared_ptr<GameClient> client);
    void transfer_client_deferred(std::shared_ptr<GameClient> client, u32 target_room_id);

private:
    std::atomic<bool> _is_running {false};

    boost::asio::any_io_executor _io_executor;
    boost::asio::strand<boost::asio::any_io_executor> _io_strand;
    boost::asio::ssl::context _ssl_context {boost::asio::ssl::context::tlsv13_server};
    tf::Executor _work_executor {};

    boost::asio::ip::tcp::acceptor _game_acceptor;
    boost::asio::ip::tcp::acceptor _admin_acceptor;

    std::shared_ptr<TcpRoom> _waiting_room;
    std::shared_ptr<SslRoom> _admin_room;
};
}