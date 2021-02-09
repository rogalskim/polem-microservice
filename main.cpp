#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "nlohmann_json/json.hpp"

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

//std::vector<const Json&> findNerLabels(const Json& labelsArray)
//{
//  assert(labelsArray.is_array());

//  return std::vector<const Json&>();
//}

//Json createPolemLabels(std::vector<const Json&> nerLabels)
//{
//  return Json();
//}

int main()
{
  auto json = readJsonFromDisk("../data/test_input.json");

  const std::string docsKey = "docs";
  const std::string labelsKey = "labels";

  if (!json.contains(docsKey))
  {
    std::cerr << "Input JSON doesn't contain \"" + docsKey + "\" key; exitting.\n";
    return 0;
  }

  const auto& docs = json.at(docsKey);
  assert(docs.is_array());

  if (docs.empty())
  {
    std::cerr << "\"" + docsKey + "\" item is empty; exitting.\n";
    return 0;
  }

  std::vector<Json> labels;
  for (const auto& doc : docs.items())
  {
    if (doc.value().is_object() && doc.value().contains(labelsKey))
    {
      const auto& labels = doc.value().at(labelsKey);
      for (const auto& label : labels.items())
        std::cout << label.value().dump() << "\n\n";
      //auto nerLabels = findNerLabels(labels);
      //auto polemLabels = createPolemLabels(nerLabels);
      //add polemLabels to doc
    }
  }

  return 0;
}
