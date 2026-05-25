#pragma once
#include <boost/asio.hpp>

boost::asio::awaitable<void> echo_session(boost::asio::ip::tcp::socket sock);
