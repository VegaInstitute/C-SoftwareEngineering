#include <atomic>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <utility>

constexpr unsigned N_REP = 1'000'000;

class Bank {
    std::map<std::string, std::atomic_int> storage;

   public:
    void add_account(const std::string& name, int balance) {
        auto& b = storage[name];
        b = balance;
    }

    void transfer(const std::string& from, const std::string& to, int amount) {
        auto& from_balance = storage[from];
        auto& to_balance = storage[to];
        from_balance -= amount;
        to_balance += amount;
    }

    void print() const {
        for (const auto& [name, balance] : storage) {
            std::cout << name << ":\t" << balance << '\n';
        }
    }
};

int main() {
    Bank bank;
    bank.add_account("Sanya", 100);
    bank.add_account("Vanya", 100);
    for (unsigned i = 0; i < N_REP; ++i) {
        std::cout << i << '\r';
        std::thread t1([&bank] { bank.transfer("Sanya", "Vanya", 10); });
        std::thread t2([&bank] { bank.transfer("Vanya", "Sanya", 10); });
        t1.join();
        t2.join();
    }
    bank.print();

    return EXIT_SUCCESS;
}
