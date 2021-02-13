#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "nlohmann_json/json.hpp"

#include "label_processing.h"

using Json = nlohmann::json;
namespace fs = std::filesystem;

Json readJsonFromDisk(const fs::path& path)
{
  if (!fs::exists(path))
    throw std::invalid_argument("File doesn't exist");
  if (path.extension() != ".json")
    throw std::invalid_argument("File is not of .json type");

  std::ifstream jsonFile(path, std::ifstream::in);

  if (!jsonFile.good())
    throw std::runtime_error("Failed to open file " + path.string());

  Json json;
  jsonFile >> json;

  jsonFile.close();

  return json;
}

int main()
{
  try
  {
    auto json = readJsonFromDisk("../data/test_input.json");
    label_processing::findAndLemmatizeNerLabelsInJson(json);
    std::cout << std::setw(4) << json << std::endl;
    return 0;
  }
  catch (const std::runtime_error& exception)
  {
    std::cout << exception.what() << "\nAborting...\n";
    return 1;
  }
}
