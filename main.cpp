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
  auto json = readJsonFromDisk("../data/test_input.json");

  if (!json.contains(key_names::docsKey))
  {
    std::cerr << "Input JSON doesn't contain \"" + key_names::docsKey + "\" key; exitting.\n";
    return 0;
  }

  const auto& docs = json.at(key_names::docsKey);
  assert(docs.is_array());

  if (docs.empty())
  {
    std::cerr << "\"" + key_names::docsKey + "\" item is empty; exitting.\n";
    return 0;
  }

  std::vector<Json> labels;
  for (const auto& [key, doc] : docs.items())
  {
    if (!doc.is_object() || !doc.contains(key_names::labelsKey))
      continue;

    const auto& labels = doc.at(key_names::labelsKey);
    const auto& nerLabels = findNerLabels(labels);
    const auto& polemLabels = lemmatizeNerLabels(nerLabels);
    //addLemmatizedLabels(&labels, polemLabels);
  }

  return 0;
}
