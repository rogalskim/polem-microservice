#include "rest_request_handler.h"

#include <iomanip>

#include "nlohmann_json/json.hpp"

#include "label_processing.h"

using namespace Pistache;

using Json = nlohmann::json;

void RestRequestHandler::onRequest(const Http::Request& request, Http::ResponseWriter response)
{
  auto requestContentHeader = request.headers().get<Http::Header::ContentType>();
  auto requestContentType = requestContentHeader->mime();

  std::cout << "> Received Request\n";
  std::cout << "  Host: " << request.address().host() << "\n";
  std::cout << "  Port: " << request.address().port() << "\n";
  std::cout << "  Method: " << request.method() << "\n";
  std::cout << "  Resource: " << request.resource() << "\n";
  std::cout << "  Content Type: " << requestContentType.raw() << "\n";
  std::cout << "  Body Length: " << request.body().size() << "\n";

  if (request.method() != Http::Method::Post)
  {
    std::cout << "> Request Rejected\n";
    response.send(Http::Code::Bad_Request, "Invalid request method; only POST is accepted.\n");
    return;
  }

  if (requestContentType != Http::Mime::MediaType::fromString("application/json"))
  {
    std::cout << "> Request Rejected\n";
    response.send(Http::Code::Unsupported_Media_Type, "Invalid request content type; \"application/json\" expected.\n");
    return;
  }

  Json json = Json::parse(request.body());
  try
  {
    label_processing::findAndLemmatizeNerLabelsInJson(json);
  }
  catch (const std::runtime_error& exception)
  {
    std::cout << "> Failed to process input JSON: " << exception.what() << "\n";
    response.send(Http::Code::Unprocessable_Entity, exception.what());
  }

  std::cout << "> Input JSON processed successfully, sending response...\n";

  response.setMime(Http::Mime::MediaType::fromString("application/json"));
  std::stringstream prettyOutputJson;
  prettyOutputJson << std::setw(2) << json << "\n";
  response.send(Http::Code::Ok, prettyOutputJson.str());

  std::cout << "> Done\n";
}
