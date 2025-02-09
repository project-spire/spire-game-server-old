#include <spire/net/connection.hpp>

using namespace spire;

bool ping_internal(net::TcpSocket& socket) {
    boost::system::error_code ec;

    // Send Ping
    {
        msg::BaseMessage base {};
        base.set_allocated_ping(new msg::Ping);
        const net::OutMessage message {base};

        boost::asio::write(socket, message.span(), ec);
        if (ec) return false;
    }

    // Receive Pong
    {
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
        if (!base.has_ping()) return false;
    }

    return true;
}

bool ping(const std::string_view host, const u16 port) {
    boost::asio::io_context io_ctx {1};
    boost::asio::ip::tcp::resolver resolver {io_ctx};
    net::TcpSocket socket {io_ctx};

    boost::system::error_code ec;
    boost::asio::connect(socket.lowest_layer(), resolver.resolve(host, std::to_string(port)), ec);
    if (ec) return false;

    const auto result {ping_internal(socket)};

    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket.close(ec);

    return result;
}

int main(const int argc, const char* argv[]) {
    if (argc != 3) EXIT_FAILURE;
    const std::string_view host {argv[1]};
    const u16 port {static_cast<u16>(std::stoi(argv[2]))};

    std::packaged_task<bool()> task {[&] {
        return ping(host, port);
    }};
    auto future {task.get_future()};
    std::thread thread {std::move(task)};

    if (future.wait_for(5s) == std::future_status::timeout) {
        thread.detach();
        return EXIT_FAILURE;
    }
    thread.join();
    return future.get() ? EXIT_SUCCESS : EXIT_FAILURE;
}