#include <string>
#include <map>
#include <iostream>
#include <thread>
#include <atomic>

const unsigned N_REP = 100'000;

class Bank {
    std::map<std::string, std::atomic_int> storage;

public:
    void set_balance(const std::string& name, int balance) {
        auto& b = storage[name];
        b = balance;
    }

    void transfer(const std::string& from, const std::string& to, int amount) {
        auto& balance_from = storage[from];
        auto& balance_to = storage[to];
        balance_from -= amount;
        balance_to += amount;
    }

    void print() const {
        for (const auto& [name, balance] : storage) {
            std::cout << name << ":\t" << balance << '\n';
        }
    }
};

int main() {
    Bank bank;
    bank.set_balance("Sanya", 100);
    bank.set_balance("Vanya", 100);
    bank.set_balance("Danya", 100);

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