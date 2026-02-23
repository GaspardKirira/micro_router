#pragma once

/**
 * @file micro_router.hpp
 * @brief Tiny HTTP router for lightweight C++ servers (header-only).
 *
 * micro_router provides:
 * - Simple route registration by HTTP method + path pattern
 * - Fast matching on path segments
 * - Named path params extraction
 *
 * Supported pattern styles:
 * - Static:      /health
 * - Colon param: /users/:id
 * - Brace param: /posts/{postId}/comments/{id}
 *
 * Notes:
 * - Matching is segment-based (split by '/')
 * - Query string is ignored during matching ("/a?x=1" matches "/a")
 * - Trailing slashes are tolerated ("/a/" matches "/a")
 */

#include <cstddef>
#include <cstdint>

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace micro_router
{
  /**
   * @brief HTTP method enum used by the router.
   */
  enum class Method : std::uint8_t
  {
    Any = 0,
    Get,
    Post,
    Put,
    Patch,
    Delete_,
    Head,
    Options
  };

  /**
   * @brief Route parameters map (name -> value).
   */
  using Params = std::unordered_map<std::string, std::string>;

  /**
   * @brief Minimal request shape used by micro_router.
   *
   * You can adapt this to your server by filling `method` + `path`.
   * When a route matches, `params` is populated.
   */
  struct Request
  {
    Method method = Method::Any;
    std::string path;
    Params params;
  };

  /**
   * @brief Minimal response shape used by micro_router.
   *
   * micro_router itself does not write headers; it only provides a place
   * for handlers to put output.
   */
  struct Response
  {
    int status = 200;
    std::string body;
  };

  /**
   * @brief Handler signature.
   */
  using Handler = std::function<void(const Request &, Response &)>;

  /**
   * @brief A matched route (internal result).
   */
  struct Match
  {
    Handler handler;
    Params params;
  };

  namespace detail
  {
    struct Segment
    {
      enum class Kind : std::uint8_t
      {
        Static = 0,
        Param
      };

      Kind kind = Kind::Static;
      std::string text; // static segment text OR param name
    };

    inline bool is_slash(char c) { return c == '/'; }

    inline std::string_view strip_query(std::string_view p)
    {
      const std::size_t q = p.find('?');
      if (q == std::string_view::npos)
        return p;
      return p.substr(0, q);
    }

    inline std::string_view trim_slashes(std::string_view p)
    {
      // remove leading slashes
      while (!p.empty() && is_slash(p.front()))
        p.remove_prefix(1);

      // remove trailing slashes
      while (!p.empty() && is_slash(p.back()))
        p.remove_suffix(1);

      return p;
    }

    inline std::vector<std::string_view> split_segments(std::string_view path)
    {
      std::vector<std::string_view> out;

      path = strip_query(path);
      path = trim_slashes(path);

      if (path.empty())
        return out;

      std::size_t i = 0;
      while (i < path.size())
      {
        const std::size_t j = path.find('/', i);
        if (j == std::string_view::npos)
        {
          out.push_back(path.substr(i));
          break;
        }
        out.push_back(path.substr(i, j - i));
        i = j + 1;
      }
      return out;
    }

    inline bool is_braced_param(std::string_view s)
    {
      return s.size() >= 3 && s.front() == '{' && s.back() == '}';
    }

    inline std::string_view unbrace(std::string_view s)
    {
      // assumes is_braced_param(s) == true
      return s.substr(1, s.size() - 2);
    }

    inline std::vector<Segment> parse_pattern(std::string_view pattern)
    {
      std::vector<Segment> segs;
      const auto parts = split_segments(pattern);

      segs.reserve(parts.size());

      for (std::string_view part : parts)
      {
        if (!part.empty() && part.front() == ':' && part.size() > 1)
        {
          segs.push_back(Segment{Segment::Kind::Param, std::string(part.substr(1))});
          continue;
        }

        if (is_braced_param(part))
        {
          const std::string_view name = unbrace(part);
          if (!name.empty())
          {
            segs.push_back(Segment{Segment::Kind::Param, std::string(name)});
            continue;
          }
          // fallthrough: treat "{}" as static if empty name
        }

        segs.push_back(Segment{Segment::Kind::Static, std::string(part)});
      }

      return segs;
    }

    inline bool method_matches(Method route_method, Method req_method)
    {
      if (route_method == Method::Any)
        return true;
      return route_method == req_method;
    }
  } // namespace detail

  /**
   * @brief Tiny router with segment-based path matching and param extraction.
   *
   * Typical usage:
   * @code
   * micro_router::Router r;
   * r.get("/health", [](auto&, auto& res){ res.body = "ok"; });
   * r.get("/users/:id", [](auto& req, auto& res){ res.body = req.params.at("id"); });
   *
   * micro_router::Request req{micro_router::Method::Get, "/users/42"};
   * micro_router::Response res;
   * if (r.dispatch(req, res)) { ... }
   * @endcode
   */
  class Router final
  {
  public:
    Router() = default;

    /**
     * @brief Add a route for a given HTTP method and pattern.
     */
    Router &add(Method method, std::string_view pattern, Handler handler)
    {
      Route r;
      r.method = method;
      r.pattern = std::string(pattern);
      r.segments = detail::parse_pattern(pattern);
      r.handler = std::move(handler);
      routes_.push_back(std::move(r));
      return *this;
    }

    /// Convenience helpers
    Router &any(std::string_view pattern, Handler handler) { return add(Method::Any, pattern, std::move(handler)); }
    Router &get(std::string_view pattern, Handler handler) { return add(Method::Get, pattern, std::move(handler)); }
    Router &post(std::string_view pattern, Handler handler) { return add(Method::Post, pattern, std::move(handler)); }
    Router &put(std::string_view pattern, Handler handler) { return add(Method::Put, pattern, std::move(handler)); }
    Router &patch(std::string_view pattern, Handler handler) { return add(Method::Patch, pattern, std::move(handler)); }
    Router &del(std::string_view pattern, Handler handler) { return add(Method::Delete_, pattern, std::move(handler)); }
    Router &head(std::string_view pattern, Handler handler) { return add(Method::Head, pattern, std::move(handler)); }
    Router &options(std::string_view pattern, Handler handler) { return add(Method::Options, pattern, std::move(handler)); }

    /**
     * @brief Try to match a request path against registered routes.
     * @return A Match if found, otherwise std::nullopt.
     */
    std::optional<Match> match(Method method, std::string_view path) const
    {
      const auto parts = detail::split_segments(path);

      for (const auto &r : routes_)
      {
        if (!detail::method_matches(r.method, method))
          continue;
        if (r.segments.size() != parts.size())
          continue;

        Params params;
        bool ok = true;

        for (std::size_t i = 0; i < r.segments.size(); ++i)
        {
          const auto &seg = r.segments[i];
          const std::string_view value = parts[i];

          if (seg.kind == detail::Segment::Kind::Static)
          {
            if (value != seg.text)
            {
              ok = false;
              break;
            }
          }
          else
          {
            // Param segment
            params.emplace(seg.text, std::string(value));
          }
        }

        if (ok)
        {
          Match m;
          m.handler = r.handler;
          m.params = std::move(params);
          return m;
        }
      }

      return std::nullopt;
    }

    /**
     * @brief Dispatches to the first matching route and calls its handler.
     *
     * - Populates req.params with extracted params.
     * - Returns true if a route matched and handler was called.
     */
    bool dispatch(Request &req, Response &res) const
    {
      std::optional<Match> m = match(req.method, req.path);
      if (!m.has_value())
        return false;

      req.params = std::move(m->params);
      m->handler(req, res);
      return true;
    }

    /**
     * @brief Number of registered routes.
     */
    std::size_t size() const noexcept { return routes_.size(); }

  private:
    struct Route
    {
      Method method = Method::Any;
      std::string pattern;
      std::vector<detail::Segment> segments;
      Handler handler;
    };

    std::vector<Route> routes_;
  };

} // namespace micro_router
