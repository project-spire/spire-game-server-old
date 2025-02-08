#include <spire/core/types.hpp>
#include <spire/net/client.hpp>

using namespace spire;

bool do_ping(const std::string_view host, const u16 port) {
    boost::asio::io_context io_ctx {1};
    // boost::asio::ssl::context ssl_ctx {boost::asio::ssl::context::tlsv13_client};
    boost::asio::ip::tcp::resolver resolver {io_ctx};
    // net::SslSocket socket {io_ctx, ssl_ctx};
    net::TcpSocket socket {io_ctx};

    //TODO: timeout

    boost::system::error_code ec;
    boost::asio::connect(socket.lowest_layer(), resolver.resolve(host, std::to_string(port)), ec);
    if (ec) return false;

    // socket.handshake(boost::asio::ssl::stream_base::client, ec);
    // if (ec) return false;

    // Send Ping
    {
        msg::BaseMessage base {};
        base.set_allocated_ping(new msg::Ping);
        net::OutMessage message {base};

        boost::asio::write(socket, message.span(), ec);
        if (ec) return false;
    }

    // Receive Pong
    {
        bool pong {false};
        while (!pong) {
            std::array<std::byte, sizeof(net::MessageHeader)> header_buffer {};
            boost::asio::read(socket, std::span {header_buffer}, ec);
            if (ec) return false;

            auto [body_size] {net::MessageHeader::deserialize(header_buffer)};
            if (body_size == 0) return false;

            std::vector<std::byte> body_buffer(body_size);
            boost::asio::read(socket, std::span {body_buffer}, ec);
            if (ec) return false;

            msg::BaseMessage base {};
            if (!base.ParseFromArray(body_buffer.data(), body_buffer.size())) return false;
            pong = base.has_ping();
        }
    }

    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket.close(ec);

    return true;
}

int main(const int argc, const char* argv[]) {
    if (argc != 3) EXIT_FAILURE;
    const std::string_view host {argv[1]};
    const u16 port {static_cast<u16>(std::stoi(argv[2]))};

    return do_ping(host, port) ? EXIT_SUCCESS : EXIT_FAILURE;
}