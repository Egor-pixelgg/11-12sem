#include "read_from.hpp"
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/as_tuple.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

asio::awaitable<std::string> read_from(tcp::socket& sock, std::string name)
{
    char buf[1024];
    auto [ec, n] = co_await sock.async_read_some(
        asio::buffer(buf),
        asio::as_tuple(asio::use_awaitable)
    );
    if (ec) co_return "[" + name + "] disconnected\n";
    co_return "[" + name + "] " + std::string(buf, n);
}
