#include "listener.hpp"
#include "echo_session.hpp"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

asio::awaitable<void> listener(unsigned short port)
{
    auto executor = co_await asio::this_coro::executor;

    tcp::acceptor acceptor(executor, tcp::endpoint(tcp::v4(), port));
    acceptor.set_option(tcp::acceptor::reuse_address(true));

    std::cout << "[*] Listening on port " << port << "\n";

    for (;;) {
        tcp::socket sock = co_await acceptor.async_accept(asio::use_awaitable);
        asio::co_spawn(executor, echo_session(std::move(sock)), asio::detached);
    }
}
