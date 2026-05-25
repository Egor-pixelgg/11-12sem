#include "multiplexer.hpp"
#include "read_from.hpp"
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <iostream>
#include <variant>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using namespace asio::experimental::awaitable_operators;

asio::awaitable<void> multiplexer(tcp::socket sock1, tcp::socket sock2)
{
    bool alive1 = true;
    bool alive2 = true;

    try {
        while (alive1 || alive2) {
            if (alive1 && alive2) {
                auto result = co_await (
                    read_from(sock1, "sock1") || read_from(sock2, "sock2")
                );
                std::visit([&](auto& val) {
                    if (val.find("disconnected") != std::string::npos) {
                        if (val.find("sock1") != std::string::npos) alive1 = false;
                        else                                          alive2 = false;
                    }
                    std::cout << val << std::flush;
                }, result);
            } else if (alive1) {
                auto msg = co_await read_from(sock1, "sock1");
                if (msg.find("disconnected") != std::string::npos) alive1 = false;
                std::cout << msg << std::flush;
            } else {
                auto msg = co_await read_from(sock2, "sock2");
                if (msg.find("disconnected") != std::string::npos) alive2 = false;
                std::cout << msg << std::flush;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[!] multiplexer: " << e.what() << "\n";
    }

    std::cout << "[*] Both sockets closed.\n";
}
