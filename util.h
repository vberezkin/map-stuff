#pragma once

#include <variant>

using E = std::exception;

template <typename F, typename... A>
auto safeTry(F&& f, A&&... a) {
  using R = decltype(f(std::forward<A>(a)...));
  using V = std::variant<R, E>;
  try {
    return V(f(std::forward<A>(a)...));
  } catch (std::exception& e) {
    return V(e);
  }
}
template <typename R>
bool isSuccess(const std::variant<R, E>& r) {
  return r.index() == 0;
}

template <typename R>
bool isFailure(const std::variant<R, E>& r) {
  return r.index() == 1;
}
