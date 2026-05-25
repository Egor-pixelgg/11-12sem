#pragma once
#include <boost/asio.hpp>
#include <cstdint>
#include <stdexcept>

namespace asio = boost::asio;

class BankAccount {
public:
    explicit BankAccount(asio::io_context& io)
        : strand_(asio::make_strand(io)), balance_(0) {}

    asio::awaitable<void> async_deposit(int64_t amount);
    asio::awaitable<void> async_withdraw(int64_t amount);
    asio::awaitable<int64_t> async_get_balance();

private:
    asio::strand<asio::io_context::executor_type> strand_;
    int64_t balance_;
};
