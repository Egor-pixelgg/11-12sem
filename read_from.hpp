#pragma once
#include <boost/asio.hpp>
#include <string>

boost::asio::awaitable<std::string> read_from(boost::asio::ip::tcp::socket& sock, std::string name);
