#include <iostream>

#include <pistache/endpoint.h>

#include "rest_request_handler.h"

using namespace Pistache;

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
