#include <iostream>
#include <string>

#include "nlohmann_json/json.hpp"

int main()
{
  nlohmann::json j;
  j["Hello!"] = "World";
  j["Year"] = 2021;
  j["NCC"] = {1,7,0,1};

  std::string serialized = j.dump();

  std::cout << serialized << "\n";

  return 0;
}
