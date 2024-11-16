#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

constexpr unsigned N_REP = 1'000'000;

class Bank {
    std::map<std::string, std::pair<int, std::mutex>> storage;

   public:
    void add_account(const std::string& name, int balance) {
        auto& [b, _] = storage[name];
        b = balance;
    }

    void transfer(const std::string& from, const std::string& to, int amount) {
        auto& [from_balance, from_mutex] = storage[from];
        auto& [to_balance, to_mutex] = storage[to];

        /*
        std::unique_lock lock1{from_mutex, std::defer_lock};
        std::unique_lock lock2{to_mutex, std::defer_lock};
        std::lock(lock1, lock2);
        */

        std::scoped_lock lock(from_mutex, to_mutex);

        from_balance -= amount;
        to_balance += amount;
    }

    void print() const {
        for (const auto& [name, data] : storage) {
            std::cout << name << ":\t" << data.first << '\n';
        }
    }
};

int main() {
    Bank bank;
    bank.add_account("Sanya", 100);
    bank.add_account("Vanya", 100);
    bank.add_account("Danya", 100);
    for (unsigned i = 0; i < N_REP; ++i) {
        std::cout << i << '\r';
        std::thread t1([&bank] { bank.transfer("Sanya", "Vanya", 10); });
        std::thread t2([&bank] { bank.transfer("Vanya", "Danya", 10); });
        std::thread t3([&bank] { bank.transfer("Danya", "Sanya", 10); });
        t1.join();
        t2.join();
        t3.join();
    }
    bank.print();

    return EXIT_SUCCESS;
}
