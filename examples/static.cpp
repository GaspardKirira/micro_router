#include <micro_router/micro_router.hpp>
#include <iostream>

int main()
{
  using namespace micro_router;

  Router router;

  router.get("/health", [](const Request &, Response &res)
             { res.body = "ok"; });

  Request req{Method::Get, "/health"};
  Response res;

  if (router.dispatch(req, res))
    std::cout << "Status: " << res.status << "\nBody: " << res.body << "\n";
  else
    std::cout << "No route matched\n";

  return 0;
}
