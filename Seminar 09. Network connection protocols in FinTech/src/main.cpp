#include <mce/generic_monte_carlo.hpp>
#include <mce/products.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <string.h>
#include <iostream>
#include <json/json.h>

#define PORT 8080
#define BUF_SIZE 1024

void handleClient(int client_sock) {
    char buffer[BUF_SIZE] = {0};
    read(client_sock, buffer, BUF_SIZE);

    // Define option parameters
    double spot = 100.0, rate = 0.05, sigma = 0.2;
    int n_simulations = 10000, n_time_steps = 365;
    double strike = 100.0;
    double maturity = 1;

    // Instantiate European Call Option and Monte Carlo Pricer
    EuropeanCallOption callOption(strike, maturity);
    MonteCarloPricer<EuropeanCallOption> pricer(n_simulations, n_time_steps, spot, rate, sigma, callOption);

    MonteCarloResult mc_result = pricer.get_result();

    // Create JSON response
    Json::Value root;
    root["option_type"] = "European Call";
    root["model_type"] = "black_scholes";
    root["price"] = mc_result.price;
    root["error"] = mc_result.std;
    root["simulations"] = n_simulations;

    Json::StreamWriterBuilder writer;
    std::string response = Json::writeString(writer, root);

    write(client_sock, response.c_str(), response.size());
    close(client_sock);
}

int main() {
    int server_fd, client_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        if ((client_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            continue;
        }
        std::cout << "Connection accepted...\n";
        handleClient(client_sock);
    }

    return 0;
}
