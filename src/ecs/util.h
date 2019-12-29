#pragma once

#include <cstddef>
#include <utility>

#include "entity.h"

namespace ecs
{
namespace util
{
template <class Tup, class Func, std::size_t... Is>
constexpr void
StaticForImpl(Tup&& t, Func&& f, std::index_sequence<Is...>)
{
  (f(std::integral_constant<std::size_t, Is>{}, std::get<Is>(t)), ...);
}

template <class... T, class Func>
constexpr void
StaticFor(std::tuple<T...>& t, Func&& f)
{
  StaticForImpl(t, std::forward<Func>(f),
                std::make_index_sequence<sizeof...(T)>{});
}

template <typename F, typename Tuple, size_t... S>
decltype(auto)
ApplyTupleImpl(F&& fn, Entity entity, Tuple&& t, std::index_sequence<S...>)
{
  return std::forward<F>(fn)(entity,
                             (std::get<S>(std::forward<Tuple>(t))->second)...);
}

template <typename F, typename Tuple>
decltype(auto)
ApplyFromTuple(F&& fn, Entity entity, Tuple&& t)
{
  std::size_t constexpr tSize =
      std::tuple_size<typename std::remove_reference<Tuple>::type>::value;
  return ApplyTupleImpl(std::forward<F>(fn), entity, std::forward<Tuple>(t),
                        std::make_index_sequence<tSize>());
}

}  // namespace util
}  // namespace ecs
