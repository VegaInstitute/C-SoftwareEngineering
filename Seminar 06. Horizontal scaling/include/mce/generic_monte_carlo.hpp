#ifndef GENERIC_MONTE_CARLO_HPP
#define GENERIC_MONTE_CARLO_HPP

#include <vector>
#include <random>
#include <thread>
#include <cmath>


struct MonteCarloResult
{
    double price = 0.0;
    double std = 0.0;
};

template <typename T>
class MonteCarloPricer {
private:
    int n_simulations;
    int n_time_steps;
    int seed;
    MonteCarloResult result;

    double spot;
    double rate;
    double sigma;

    T product;

    // Make sure to pass this as a pointer to member function
    void thread_price(int num_of_simulations, MonteCarloResult& mc_result);
    void price();

public:
    MonteCarloPricer(int n_simulations, int n_time_steps, double spot, double rate, double sigma, T product)
        : n_simulations(n_simulations), n_time_steps(n_time_steps), spot(spot), rate(rate), sigma(sigma), product(product) {
        // TODO check n_simulation > 0, etc
        price();
    };

    MonteCarloResult get_result() { return result; };

};

template <typename T>
void MonteCarloPricer<T>::thread_price(int num_of_simulations, MonteCarloResult& mc_result) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(0., 1.);

    for (int j = 0; j < num_of_simulations; ++j) {
        double log_spot = log(spot);
        double dt = product.get_maturity() / n_time_steps;

        for(int i = 0; i < n_time_steps; ++i) {
            double z = d(gen);
            log_spot = log_spot + (rate - 0.5 * sigma * sigma) * dt + sigma * sqrt(dt) * z;
        }

        double payoff = product.payoff(exp(log_spot));
        mc_result.price += payoff;
        mc_result.std += payoff * payoff;
    }

    mc_result.price /= num_of_simulations;
    mc_result.std /= num_of_simulations;
    mc_result.std -= mc_result.price * mc_result.price;
    mc_result.std = sqrt(mc_result.std);
}

template <typename T>
void MonteCarloPricer<T>::price() {
    int num_threads = std::thread::hardware_concurrency();
    int n_simulations_thread = n_simulations / num_threads;

    std::vector<std::thread> threads;
    std::vector<MonteCarloResult> results(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        // Use a lambda to capture `this` and pass the member function
        threads.emplace_back([this, i, n_simulations_thread, &results]() {
            this->thread_price(n_simulations_thread, results[i]);
        });
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Aggregate results
    result.price = 0;
    result.std = 0;
    for (auto& r : results) {
        result.price += r.price;
        result.std += r.std * r.std;
    }

    result.price /= num_threads;
    result.std = sqrt(result.std / num_threads);
}

#endif // GENERIC_MONTE_CARLO_HPP
