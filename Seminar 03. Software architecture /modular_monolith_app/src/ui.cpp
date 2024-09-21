#include "ui.h"
#include <iostream>

bool Response::get_data(double &data) const {
    if (!code) {
        data = this->data;
        return true;
    }
    return false;
}

Response user_input(std::string prompt) {
    double number;
    std::cout << prompt;
    std::cin >> number;
    return Response(number, 0);
}