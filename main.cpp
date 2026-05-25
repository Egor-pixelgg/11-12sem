#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/as_tuple.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <random>
#include <vector>
#include <memory>

#include "listener.hpp"
#include "multiplexer.hpp"
#include "bank_account.hpp"

namespace asio = boost::asio;
using tcp = asio::ip::tcp;
asio::awaitable<void> simple_server(unsigned short port, std::string label)
{
    auto ex = co_await asio::this_coro::executor;
    tcp::acceptor acc(ex, tcp::endpoint(tcp::v4(), port));
    acc.set_option(tcp::acceptor::reuse_address(true));

    std::cout << "[" << label << "] listening on port " << port << "\n";
    std::cout << "[" << label << "] type messages and press Enter\n\n";

    tcp::socket client = co_await acc.async_accept(asio::use_awaitable);
    std::cout << "[" << label << "] client connected\n";

    asio::steady_timer idle(ex);

    std::thread stdin_thread([&]() {
        std::string line;
        while (std::getline(std::cin, line)) {
            line += "\n";
            asio::post(ex, [&client, line, &idle]() {
                asio::async_write(client, asio::buffer(line), [](auto, auto) {});
                idle.cancel();
            });
        }
        asio::post(ex, [&idle]() { idle.cancel(); });
    });

    try {
        for (;;) {
            idle.expires_at(asio::steady_timer::time_point::max());
            auto [ec] = co_await idle.async_wait(asio::as_tuple(asio::use_awaitable));
            if (!client.is_open()) break;
        }
    } catch (...) {}

    stdin_thread.detach();
}

asio::awaitable<void> task2_run()
{
    auto ex = co_await asio::this_coro::executor;

    asio::steady_timer delay(ex, std::chrono::milliseconds(200));
    co_await delay.async_wait(asio::use_awaitable);

    tcp::socket s1(ex), s2(ex);
    co_await s1.async_connect(
        tcp::endpoint(asio::ip::make_address("127.0.0.1"), 13001),
        asio::use_awaitable
    );
    co_await s2.async_connect(
        tcp::endpoint(asio::ip::make_address("127.0.0.1"), 13002),
        asio::use_awaitable
    );

    std::cout << "[*] Connected to both servers. Starting multiplexer.\n\n";
    co_await multiplexer(std::move(s1), std::move(s2));
}
struct Task3State {
    BankAccount                                                  account;
    std::atomic<int64_t>                                         total_deposited{0};
    std::atomic<int64_t>                                         total_withdrawn{0};
    std::atomic<int>                                             done{0};
    int                                                          total;
    asio::executor_work_guard<asio::io_context::executor_type>   guard;

    Task3State(asio::io_context& io, int n)
        : account(io), total(n), guard(asio::make_work_guard(io)) {}
};

asio::awaitable<void> worker(std::shared_ptr<Task3State> state)
{
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int64_t> dist(1, 100);

    for (int i = 0; i < 10; ++i) {
        int64_t amount = dist(rng);
        co_await state->account.async_deposit(amount);
        state->total_deposited.fetch_add(amount);
    }

    for (int i = 0; i < 10; ++i) {
        int64_t amount = dist(rng);
        try {
            co_await state->account.async_withdraw(amount);
            state->total_withdrawn.fetch_add(amount);
        } catch (const std::invalid_argument&) {
        }
    }

    if (++(state->done) == state->total) {
        int64_t balance  = co_await state->account.async_get_balance();
        int64_t expected = state->total_deposited.load() - state->total_withdrawn.load();
        std::cout << "Total deposited : " << state->total_deposited.load() << "\n";
        std::cout << "Total withdrawn : " << state->total_withdrawn.load() << "\n";
        std::cout << "Expected balance: " << expected << "\n";
        std::cout << "Actual  balance : " << balance  << "\n";
        std::cout << (balance == expected ? "[OK] Match!" : "[FAIL] Mismatch!") << "\n";
        state->guard.reset();
    }
}

static void run_task1(asio::io_context& io)
{
    std::cout << "[task1] Echo server on port 12345\n";
    std::cout << "[task1] Connect with nc or telnet to 127.0.0.1:12345\n\n";
    asio::co_spawn(io, listener(12345), asio::detached);
}

static void run_server1(asio::io_context& io)
{
    asio::co_spawn(io, simple_server(13001, "server1"), asio::detached);
}

static void run_server2(asio::io_context& io)
{
    asio::co_spawn(io, simple_server(13002, "server2"), asio::detached);
}

static void run_task2(asio::io_context& io)
{
    std::cout << "[task2] Connecting to server1:13001 and server2:13002\n\n";
    asio::co_spawn(io, task2_run(), asio::detached);
}

static void run_task3(asio::io_context& io)
{
    constexpr int N = 100;
    std::cout << "[task3] Starting " << N << " concurrent workers (4 threads)\n\n";

    auto state = std::make_shared<Task3State>(io, N);

    for (int i = 0; i < N; ++i)
        asio::co_spawn(io, worker(state), asio::detached);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <mode>\n"
                  << "  task1    - TCP echo server (port 12345)\n"
                  << "  server1  - simple server for task2 (port 13001)\n"
                  << "  server2  - simple server for task2 (port 13002)\n"
                  << "  task2    - multiplexer (connects to server1 and server2)\n"
                  << "  task3    - bank account with strand (4 threads, 100 workers)\n";
        return 1;
    }

    std::string task = argv[1];

    try {
        asio::io_context io;

        asio::signal_set signals(io, SIGINT);
        signals.async_wait([&](auto, auto) {
            std::cout << "\n[*] Stopping...\n";
            io.stop();
        });

        if      (task == "task1")   run_task1(io);
        else if (task == "server1") run_server1(io);
        else if (task == "server2") run_server2(io);
        else if (task == "task2")   run_task2(io);
        else if (task == "task3")   run_task3(io);
        else {
            std::cerr << "[!] Unknown mode: " << task << "\n";
            return 1;
        }

        if (task == "task3") {
            std::vector<std::thread> threads;
            for (int i = 0; i < 4; ++i)
                threads.emplace_back([&io]() { io.run(); });
            for (auto& t : threads)
                t.join();
        } else {
            io.run();
        }

    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
        return 1;
    }

    return 0;
}
