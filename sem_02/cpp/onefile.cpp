#include <cstdlib>
#include <iostream>
#include <stdexcept>

class BSModelParams {
  double vola;

public:
  BSModelParams(double volatility) { this->vola = volatility; }

  double total_variance(double dt_years) const {
    return this->vola * this->vola * dt_years;
  }
};

int main() {

  double vol, dt;

  std::cout << "Volatility: ";
  std::cin >> vol;
  std::cout << "Time span in years: ";
  std::cin >> dt;
  std::cout << "Total variance is " << BSModelParams(vol).total_variance(dt)
            << '\n';

  return EXIT_SUCCESS;
}
