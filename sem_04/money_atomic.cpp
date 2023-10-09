#include <atomic>
#include <iostream>
#include <map>
#include <string>
#include <thread>

class AccountsDB {
    std::map<std::string, std::atomic_int> storage;

   public:
    void add_account(const std::string &name, int balance) {
        storage[name] = balance;
    }

    friend std::ostream &operator<<(std::ostream &os, const AccountsDB &adb) {
        for (const auto &it : adb.storage) {
            auto &[name, balance] = it;
            os << name << ":\t" << balance << '\n';
        }
        return os;
    }

    void transfer(const std::string &from, const std::string &to, int amount) {
        auto &from_balance = storage.at(from);
        auto &to_balance = storage.at(to);
        from_balance -= amount;
        to_balance += amount;
    }
};

int main() {
    constexpr unsigned N_REP = 100'000;

    AccountsDB adb;
    adb.add_account("Vanya", 100);
    adb.add_account("Sanya", 100);

    for (unsigned i = 0; i < N_REP; ++i) {
        std::jthread t1([&adb]() { adb.transfer("Vanya", "Sanya", 10); });
        std::jthread t2([&adb]() { adb.transfer("Sanya", "Vanya", 10); });
    }
    std::cout << adb << '\n';

    return 0;
}
