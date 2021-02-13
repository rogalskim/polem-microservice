#define BOOST_TEST_MODULE json_parsing_tests

#include <iostream>

#include <boost/test/included/unit_test.hpp>

#include <polem-dev/CascadeLemmatizer.h>

#include "../nlohmann_json/json.hpp"
#include "../label_processing.h"

using Json = nlohmann::json;
using namespace label_processing;

BOOST_AUTO_TEST_SUITE(ner_finding_tests)

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

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(tag_value_list_builder_tests)

BOOST_AUTO_TEST_CASE(buildTagValueList_returns_vector_of_strings_of_size_equal_to_posTag_label_count)
{
  auto testJson =
    R"({
      "labels":
       [
        {
          "startToken": 0,
          "endToken": 1,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "adj:sg:nom:f:pos"
        },
        {
          "startToken": 1,
          "endToken": 2,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "spam:spam:eggs:spam"
        },
        {"fieldName": "lemmas"},
        {"fieldName": "NER"}
       ]
    })"_json;

  std::vector<std::string> posTagValueList
      = label_processing::buildTagValueList("posTag", testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL(posTagValueList.size(), 2u);
}

BOOST_AUTO_TEST_CASE(buildTagValueList_throws_if_there_is_a_posTag_label_missing)
{
  auto testJson =
    R"({
      "labels":
       [
        {
          "startToken": 0,
          "endToken": 1,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "FIRST TAG"
        },
        {
          "startToken": 1,
          "endToken": 2,
          "fieldName": ">> NOT POSTAG <<",
          "serviceName": "EGGS",
          "value": "SECOND TAG"
        },
        {
          "startToken": 2,
          "endToken": 3,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "THIRD TAG"
        }
       ]
    })"_json;

  BOOST_CHECK_THROW(label_processing::buildTagValueList("posTag", testJson.at(key_names::labelsKey)),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(buildTagValueList_expects_exactly_1_posTag_per_token_otherwise_thorws)
{
  auto testJson =
    R"({
      "labels":
       [
        {
          "startToken": 0,
          "endToken": 3,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "endToken - startToken > 1"
        }
       ]
    })"_json;

  BOOST_CHECK_THROW(label_processing::buildTagValueList("posTag", testJson.at(key_names::labelsKey)),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(buildTagValueList_returns_list_of_found_posTag_values)
{
  std::vector<Json> posTagList =
  {
    R"({
          "startToken": 0,
          "endToken": 1,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "FIRST TAG"
      })"_json,
    R"({
          "startToken": 1,
          "endToken": 2,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "SECOND TAG"
      })"_json,
    R"({
          "startToken": 2,
          "endToken": 3,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "THIRD TAG"
      })"_json
  };
  std::vector<std::string> expectedPosTagValues;
  auto testJson =R"({"labels": []})"_json;
  for (const auto& testPosTag : posTagList)
  {
    expectedPosTagValues.push_back(testPosTag.at("value"));
    testJson.at("labels").push_back(testPosTag);
  }

  auto outputPosTagValues = label_processing::buildTagValueList("posTag",
                                                                testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL_COLLECTIONS(expectedPosTagValues.begin(), expectedPosTagValues.end(),
                                outputPosTagValues.begin(), outputPosTagValues.end());
}

BOOST_AUTO_TEST_CASE(buildTagValueList_returns_posTag_list_sorted_by_startToken)
{
  std::vector<Json> posTagList =
  {
    R"({
          "startToken": 0,
          "endToken": 1,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "FIRST TAG"
      })"_json,
    R"({
          "startToken": 1,
          "endToken": 2,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "SECOND TAG"
      })"_json,
    R"({
          "startToken": 2,
          "endToken": 3,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "THIRD TAG"
      })"_json
  };

  std::vector<std::string> expectedPosTagValues;
  for (const auto& testPosTag : posTagList)
    expectedPosTagValues.push_back(testPosTag.at("value"));

  auto testJson =R"({"labels": []})"_json;
  testJson.at(key_names::labelsKey).push_back(posTagList[1]);
  testJson.at(key_names::labelsKey).push_back(posTagList[2]);
  testJson.at(key_names::labelsKey).push_back(posTagList[0]);

  auto outputPosTagList = label_processing::buildTagValueList("posTag",
                                                              testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL_COLLECTIONS(expectedPosTagValues.begin(), expectedPosTagValues.end(),
                                outputPosTagList.begin(), outputPosTagList.end());
}

BOOST_AUTO_TEST_CASE(buildTagValueList_can_build_both_posTag_and_lemmas_lists)
{
  std::vector<Json> posTagList =
  {
    R"({
          "startToken": 0,
          "endToken": 1,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "FIRST TAG"
      })"_json,
    R"({
          "startToken": 1,
          "endToken": 2,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "SECOND TAG"
      })"_json
  };

  std::vector<Json> lemmaTagList =
  {
    R"({
          "startToken": 0,
          "endToken": 1,
          "fieldName": "lemmas",
          "serviceName": "tagger",
          "value": ["FIRST LEMMA"]
      })"_json,
    R"({
          "startToken": 1,
          "endToken": 2,
          "fieldName": "lemmas",
          "serviceName": "tagger",
          "value": ["SECOND LEMMA"]
      })"_json
  };

  std::vector<std::string> expectedPosTagValues;
  auto testJson =R"({"labels": []})"_json;
  for (const auto& testPosTag : posTagList)
  {
    expectedPosTagValues.push_back(testPosTag.at("value"));
    testJson.at("labels").push_back(testPosTag);
  }
  std::vector<std::string> expectedLemmaValues;
  for (const auto& testLemma : lemmaTagList)
  {
    expectedLemmaValues.push_back(testLemma.at("value")[0]);
    testJson.at("labels").push_back(testLemma);
  }

  auto outputPosTagValues
      = label_processing::buildTagValueList("posTag", testJson.at(key_names::labelsKey));
  auto outputLemmaTagValues
      = label_processing::buildTagValueList("lemmas", testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL_COLLECTIONS(expectedPosTagValues.begin(), expectedPosTagValues.end(),
                                outputPosTagValues.begin(), outputPosTagValues.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(expectedLemmaValues.begin(), expectedLemmaValues.end(),
                                outputLemmaTagValues.begin(), outputLemmaTagValues.end());
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(lemmatization_tests)

BOOST_AUTO_TEST_CASE(lemmatizeNerLabels_throws_if_posTag_and_lemmaTag_sizes_not_equal)
{
  auto testJson =
    R"({
      "labels":
       [
        {
          "startToken": 0,
          "endToken": 1,
          "fieldName": "lemmas",
          "value": ["FIRST LEMMA"]
        },
        {
          "startToken": 0,
          "endToken": 1,
          "fieldName": "posTag",
          "value": "FIRST POSTAG"
        },
        {
          "startToken": 1,
          "endToken": 2,
          "fieldName": "lemmas",
          "value": ["SECOND LEMMA"]
        }
       ]
    })"_json;

  auto labelArray = testJson.at(key_names::labelsKey);
  auto nerLabels = label_processing::findNerLabels(labelArray);
  auto posTagValues = label_processing::buildTagValueList("posTag", labelArray);
  auto lemmaTagValues = label_processing::buildTagValueList("lemmas", labelArray);
  CascadeLemmatizer lemmatizer = CascadeLemmatizer::assembleLemmatizer();

  BOOST_CHECK_THROW(label_processing::lemmatizeNerLabels(nerLabels,
                                                         posTagValues,
                                                         lemmaTagValues,
                                                         lemmatizer),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(string_builder_throws_if_cant_find_posTag_needed_for_a_NER_label)
{
  auto testJson =
    R"({
      "labels":
       [
        {
          "startToken": 0,
          "endToken": 1,
          "fieldName": "namedEntityML",
          "serviceName": "NER",
          "value": "Alejach Jerozolimskich"
        },
        {
          "startToken": 0,
          "endToken": 1,
          "fieldName": "lemmas",
          "value": ["na"]
        },
        {
          "startToken": 0,
          "endToken": 1,
          "fieldName": "posTag",
          "value": "prep:loc"
        }
       ]
    })"_json;

  auto labelArray = testJson.at(key_names::labelsKey);
  auto nerLabels = label_processing::findNerLabels(labelArray);
  auto posTagValues = label_processing::buildTagValueList("posTag", labelArray);
  auto lemmaTagValues = label_processing::buildTagValueList("lemmas", labelArray);

  BOOST_CHECK_THROW(label_processing::buildPosAndLemmaStringsForNerLabel(
                      nerLabels[0], posTagValues, lemmaTagValues),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(string_builder_returns_correct_posTag_and_lemma_strings)
{
  auto testJson =
    R"({
      "labels":
       [
        {
          "startToken": 0,
          "endToken": 1,
          "fieldName": "namedEntityML",
          "serviceName": "NER",
          "value": "Alejach Jerozolimskich"
        },
        {
          "startToken": 0,
          "endToken": 1,
          "fieldName": "lemmas",
          "value": ["LEMMA_0"]
        },
        {
          "startToken": 0,
          "endToken": 1,
          "fieldName": "posTag",
          "value": "POSTAG_0"
        },
        {
          "startToken": 1,
          "endToken": 2,
          "fieldName": "lemmas",
          "value": ["LEMMA_1"]
        },
        {
          "startToken": 1,
          "endToken": 2,
          "fieldName": "posTag",
          "value": "POSTAG_1"
        }
       ]
    })"_json;

  auto labelArray = testJson.at(key_names::labelsKey);
  auto nerLabels = label_processing::findNerLabels(labelArray);
  auto posTagValues = label_processing::buildTagValueList("posTag", labelArray);
  auto lemmaTagValues = label_processing::buildTagValueList("lemmas", labelArray);
  std::string expectedPosTag = "postag_0 postag_1";
  std::string expectedLemma = "lemma_0 lemma_1";

  auto result = label_processing::buildPosAndLemmaStringsForNerLabel(
        nerLabels[0], posTagValues, lemmaTagValues);

  BOOST_CHECK_EQUAL(std::get<0>(result), expectedPosTag);
  BOOST_CHECK_EQUAL(std::get<1>(result), expectedLemma);
}

BOOST_AUTO_TEST_CASE(lemmatizeNerLabel_lemmatizes_label_correctly)
{
  auto inputNer = R"({
                      "end": 25,
                      "endToken": 2,
                      "fieldName": "namedEntityML",
                      "name": "sys.Localization",
                      "score": 1.0,
                      "serviceName": "NER",
                      "start": 3,
                      "startToken": 1,
                      "value": "Alejach Jerozolimskich"
                     })"_json;

  auto expectedNer = R"({
                         "end": 25,
                         "endToken": 2,
                         "fieldName": "polem",
                         "name": "polem",
                         "score": 1.0,
                         "serviceName": "Polem",
                         "start": 3,
                         "startToken": 1,
                         "value": "aleje jerozolimskie"
                        })"_json;

  auto inputPosTags = "subst:pl:loc:f adj:pl:loc:f:pos";
  auto inputLemmaTags = "aleja jerozolimski";
  CascadeLemmatizer lemmatizer = CascadeLemmatizer::assembleLemmatizer();

  auto outputNer
      = label_processing::lemmatizeNerLabel(inputNer, inputPosTags, inputLemmaTags, lemmatizer);

  BOOST_TEST(outputNer == expectedNer);
}

BOOST_AUTO_TEST_CASE(lemmatizeNerLabels_returns_correct_lemmatized_NER_labels)
{
  auto testJson =
    R"({
        "labels": [
          {
            "end": 25,
            "endToken": 2,
            "fieldName": "namedEntityML",
            "name": "sys.Settlement",
            "score": 1.0,
            "serviceName": "NER",
            "start": 11,
            "startToken": 2,
            "value": "Jerozolimskich"
          },
          {
            "end": 25,
            "endToken": 2,
            "fieldName": "namedEntityML",
            "name": "sys.Localization",
            "score": 1.0,
            "serviceName": "NER",
            "start": 3,
            "startToken": 1,
            "value": "Alejach Jerozolimskich"
          },
          {
            "end": 2,
            "endToken": 1,
            "fieldName": "lemmas",
            "name": "lemmas",
            "score": 1,
            "serviceName": "tagger",
            "start": 0,
            "startToken": 0,
            "value": [
              "na"
            ]
          },
          {
            "end": 2,
            "endToken": 1,
            "fieldName": "posTag",
            "name": "posTag",
            "score": 1,
            "serviceName": "tagger",
            "start": 0,
            "startToken": 0,
            "value": "prep:loc"
          },
          {
            "end": 10,
            "endToken": 2,
            "fieldName": "lemmas",
            "name": "lemmas",
            "score": 1,
            "serviceName": "tagger",
            "start": 3,
            "startToken": 1,
            "value": [
              "Aleja"
            ]
          },
          {
            "end": 10,
            "endToken": 2,
            "fieldName": "posTag",
            "name": "posTag",
            "score": 1,
            "serviceName": "tagger",
            "start": 3,
            "startToken": 1,
            "value": "subst:pl:loc:f"
          },
          {
            "end": 25,
            "endToken": 3,
            "fieldName": "lemmas",
            "name": "lemmas",
            "score": 1,
            "serviceName": "tagger",
            "start": 11,
            "startToken": 2,
            "value": [
              "Jerozolimski"
            ]
          },
          {
            "end": 25,
            "endToken": 3,
            "fieldName": "posTag",
            "name": "posTag",
            "score": 1,
            "serviceName": "tagger",
            "start": 11,
            "startToken": 2,
            "value": "adj:pl:loc:f:pos"
          }
        ]})"_json;

  std::vector<Json> expectedLemmatizedLabels;
  expectedLemmatizedLabels.push_back(
    R"({
        "end": 25,
        "endToken": 2,
        "fieldName": "polem",
        "name": "polem",
        "score": 1.0,
        "serviceName": "Polem",
        "start": 11,
        "startToken": 2,
        "value": "jerozolimscy"
      })"_json);
  expectedLemmatizedLabels.push_back(
    R"({
        "end": 25,
        "endToken": 2,
        "fieldName": "polem",
        "name": "polem",
        "score": 1.0,
        "serviceName": "Polem",
        "start": 3,
        "startToken": 1,
        "value": "aleje jerozolimskie"
       })"_json);

  auto labelArray = testJson.at(key_names::labelsKey);
  auto nerLabels = label_processing::findNerLabels(labelArray);
  auto posTagValues = label_processing::buildTagValueList("posTag", labelArray);
  auto lemmaTagValues = label_processing::buildTagValueList("lemmas", labelArray);
  CascadeLemmatizer lemmatizer = CascadeLemmatizer::assembleLemmatizer();

  auto lemmatizedLabels
      = label_processing::lemmatizeNerLabels(nerLabels, posTagValues, lemmaTagValues, lemmatizer);

  BOOST_CHECK_EQUAL(expectedLemmatizedLabels, lemmatizedLabels);
}

BOOST_AUTO_TEST_SUITE_END()

