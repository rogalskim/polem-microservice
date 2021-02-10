#define BOOST_TEST_MODULE json_parsing_tests

#include <iostream>

#include <boost/test/included/unit_test.hpp>

#include "../nlohmann_json/json.hpp"
#include "../label_processing.h"

using Json = nlohmann::json;
using namespace label_processing;

BOOST_AUTO_TEST_CASE(given_lables_object__ner_label_finder_returns_correct_number_of_labels)
{
  auto testJson =
    R"({
      "labels":
       [
        {"serviceName": "NER"},
        {"serviceName": "NER"}
       ]
    })"_json;

  auto nerLabels = findNerLabels(testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL(nerLabels.size(), 2u);
}

BOOST_AUTO_TEST_CASE(ner_label_finder_ignores_labels_without_ner_type)
{
  auto testJson =
    R"({
      "labels":
       [
        {"serviceName": "tagger"},
        {"serviceName": "NER"},
        {"serviceName": "other_type"}
       ]
    })"_json;

  auto nerLabels = findNerLabels(testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL(nerLabels.size(), 1u);
}

BOOST_AUTO_TEST_CASE(ner_label_finder_ignores_labels_without_serviceName_key)
{
  auto testJson =
    R"({
      "labels":
       [
        {"different_key": "Hello!"},
        {}
       ]
    })"_json;

  auto nerLabels = findNerLabels(testJson.at(key_names::labelsKey));

  BOOST_TEST(nerLabels.empty());
}

BOOST_AUTO_TEST_CASE(ner_label_finder_returns_correct_labels)
{
  auto testNerLabel =
    R"({
        "end": 6,
        "endToken": 0,
        "fieldName": "namedEntityML",
        "name": "sys.Country",
        "score": 1.0,
        "serviceName": "NER",
        "start": 0,
        "startToken": 0,
        "value": "Polska"
      })"_json;

  auto testJson =
    R"({
      "labels":
       [
        {
          "end": 6,
          "endToken": 1,
          "fieldName": "lemmas",
          "name": "lemmas",
          "score": 1,
          "serviceName": "tagger",
          "start": 0,
          "startToken": 0,
          "value": [
            "polski"
          ]
        }
       ]
    })"_json;

  testJson.at(key_names::labelsKey).push_back(testNerLabel);

  auto nerLabels = findNerLabels(testJson.at(key_names::labelsKey));

  BOOST_REQUIRE_EQUAL(nerLabels.size(), 1u);
  BOOST_TEST(nerLabels[0] == testNerLabel, "Returned label is not equal to expected label");
}
