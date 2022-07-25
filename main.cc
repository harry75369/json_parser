#include "json.hpp"
#include <iostream>
#include <string>

int main() {
  std::string line, buff;
  while (std::cin >> line) {
    buff += "\n";
    buff += line;
  }
  try {
    auto [json, _] = Json::parse(buff);
    Json::print(json);
  } catch (const char *exp) {
    std::cout << exp << std::endl;
    return 1;
  }
  return 0;
}
