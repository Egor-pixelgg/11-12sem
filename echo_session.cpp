#include "echo_session.hpp"
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>
#include <iostream>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

asio::awaitable<void> echo_session(tcp::socket sock)
{
    auto remote = sock.remote_endpoint();
    std::cout << "[+] " << remote << "\n";

    char data[1024];
    try {
        for (;;) {
            auto [ec, n] = co_await sock.async_read_some(
                asio::buffer(data),
                asio::as_tuple(asio::use_awaitable)
            );
            if (ec == asio::error::eof) {
                std::cout << "[-] " << remote << "\n";
                break;
            }
            if (ec) throw boost::system::system_error(ec);
            co_await asio::async_write(sock, asio::buffer(data, n), asio::use_awaitable);
        }
    } catch (const std::exception& e) {
        std::cerr << "[!] " << e.what() << "\n";
    }
}
