#include <iostream>
#include <string>

// C++11

// s1 - std::string
// std::move(s1) - std::string&&

class String {
  size_t len;
  char *str_begin;

public:
  String() : len{0}, str_begin{nullptr} {}

  String(const String &other) {
    // allocate memory for sting calloc(...)
    // len = other.len;
    // copy each symbol in a loop
  }

  String(String &&other) {
    len = other.len;
    str_begin = other.str_begin;
    other.str_begin = nullptr;
    other.len = 0;
  }
};

int main() {
  std::string s1 = "Hello, World!";
  std::string s2;
  s2 = s1;
  s2 = std::move(s1);
  std::cout << s1 << '\n';
  std::cout << s2 << '\n';

  // C++17
  // std::string_view -- does not owe memory
}
