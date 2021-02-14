#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <pistache/endpoint.h>
#include <pistache/net.h>

#include "label_processing.h"

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

using namespace Pistache;

class RestRequestHandler : public Http::Handler
{
public:
  HTTP_PROTOTYPE(RestRequestHandler)

  void onRequest(const Http::Request& request, Http::ResponseWriter response) override
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
};

int main()
{
  std::cout << "> Starting the server...\n";

  Address address(Ipv4::any(), Port(5000));

  const int serverThreadCount = 1;
  const int maxRequestBytes = 1024*1024;
  const int maxResponseBytes = 1024*1024;
  auto options = Http::Endpoint::options()
      .threads(serverThreadCount)
      .maxRequestSize(maxRequestBytes)
      .maxResponseSize(maxResponseBytes);

  Http::Endpoint server(address);
  server.init(options);
  server.setHandler(Http::make_handler<RestRequestHandler>());

  std::cout << "> Ready to serve!\n";
  server.serve();

  return 0;
}
