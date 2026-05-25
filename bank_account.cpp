#include "bank_account.hpp"

asio::awaitable<void> BankAccount::async_deposit(int64_t amount)
{
    co_await asio::post(strand_, asio::use_awaitable);
    balance_ += amount;
}

asio::awaitable<void> BankAccount::async_withdraw(int64_t amount)
{
    co_await asio::post(strand_, asio::use_awaitable);
    if (amount > balance_)
        throw std::invalid_argument("Insufficient funds");
    balance_ -= amount;
}

asio::awaitable<int64_t> BankAccount::async_get_balance()
{
    co_await asio::post(strand_, asio::use_awaitable);
    co_return balance_;
}
