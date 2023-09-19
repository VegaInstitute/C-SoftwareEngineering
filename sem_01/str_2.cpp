#include <iostream>
#include <string>

// C++98

int main() {
  std::string s1 = "Hello, World!";
  std::string s2;
  s2 = s1;
  // std::string &s2 = s1;
  s1[0] = 'h';
  std::cout << s1 << '\n';
  std::cout << s2 << '\n';
}
