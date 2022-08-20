#define HOMEBREW
#ifdef HOMEBREW
#include "json.hpp"
#else
#include "nlohmann/json.hpp"
#endif
#include <iostream>
#include <string>

int main() {
  std::string line, buff;
  while (std::getline(std::cin, line)) {
    buff += "\n";
    buff += line;
  }
#ifdef HOMEBREW
  try {
    auto [json, _] = Json::parse(buff);
    Json::print(json);
  } catch (const char *exp) {
    std::cout << exp << std::endl;
    return 1;
  }
#else
  try {
    auto json = nlohmann::json::parse(buff);
    std::cout << json.dump(2);
  } catch (const nlohmann::json::exception& exp) {
    std::cout << exp.what() << std::endl;
    return 1;
  }
#endif
  return 0;
}
