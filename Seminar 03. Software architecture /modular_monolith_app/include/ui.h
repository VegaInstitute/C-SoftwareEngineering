#ifndef UI_H
#define UI_H

#include <string>

class Response {
private:
    double data;
    int code;
public:
    Response(double data, int code) : data(data), code(code) {}
    bool get_data(double &data) const;
};

Response user_input(std::string prompt);

#endif // UI_H