#pragma once

#include <variant>

using E = std::exception;

template <typename R, typename F, typename... A>
std::variant<R, E> safeTry(F&& f, A&&... a) {
  try {
    return f(std::forward<A>(a)...);
  } catch (std::exception& e) {
    return e;
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
