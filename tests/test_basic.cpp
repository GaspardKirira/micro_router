#include <micro_router/micro_router.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

static void expect(bool ok, const char *msg)
{
  if (!ok)
  {
    std::cerr << "Test failed: " << msg << "\n";
    std::exit(1);
  }
}

int main()
{
  using namespace micro_router;

  Router r;

  r.get("/health", [](const Request &, Response &res)
        {
    res.status = 200;
    res.body = "ok"; });

  r.get("/users/:id", [](const Request &req, Response &res)
        {
    auto it = req.params.find("id");
    expect(it != req.params.end(), "param id should exist");
    res.status = 200;
    res.body = std::string("user=") + it->second; });

  r.get("/posts/{postId}/comments/{id}", [](const Request &req, Response &res)
        {
    expect(req.params.at("postId") == "7", "postId should be 7");
    expect(req.params.at("id") == "99", "id should be 99");
    res.status = 200;
    res.body = "ok"; });

  // 1) static route
  {
    Request req{Method::Get, "/health"};
    Response res;
    const bool dispatched = r.dispatch(req, res);

    expect(dispatched, "health route should dispatch");
    expect(res.status == 200, "health status should be 200");
    expect(res.body == "ok", "health body should be ok");
  }

  // 2) param route (also ignore query string)
  {
    Request req{Method::Get, "/users/42?x=1"};
    Response res;
    const bool dispatched = r.dispatch(req, res);

    expect(dispatched, "users/:id route should dispatch");
    expect(req.params.at("id") == "42", "id should be 42");
    expect(res.body == "user=42", "body should include id");
  }

  // 3) braced params route + trailing slash tolerance
  {
    Request req{Method::Get, "/posts/7/comments/99/"};
    Response res;
    const bool dispatched = r.dispatch(req, res);

    expect(dispatched, "braced params route should dispatch");
    expect(res.status == 200, "status should be 200");
  }

  // 4) method mismatch
  {
    Request req{Method::Post, "/health"};
    Response res;
    const bool dispatched = r.dispatch(req, res);

    expect(!dispatched, "POST /health should not dispatch (only GET registered)");
  }

  // 5) no match
  {
    Request req{Method::Get, "/nope"};
    Response res;
    const bool dispatched = r.dispatch(req, res);

    expect(!dispatched, "unknown route should not dispatch");
  }

  std::cout << "micro_router: all tests passed\n";
  return 0;
}
