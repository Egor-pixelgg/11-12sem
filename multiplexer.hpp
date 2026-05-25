#pragma once
#include <boost/asio.hpp>

boost::asio::awaitable<void> multiplexer(boost::asio::ip::tcp::socket sock1, boost::asio::ip::tcp::socket sock2);
