#include <micro_router/micro_router.hpp>
#include <iostream>

int main()
{
  using namespace micro_router;

  Router router;

  router.get("/users/:id", [](const Request &req, Response &res)
             { res.body = "User id = " + req.params.at("id"); });

  Request req{Method::Get, "/users/42"};
  Response res;

  router.dispatch(req, res);

  std::cout << res.body << "\n";
  return 0;
}
