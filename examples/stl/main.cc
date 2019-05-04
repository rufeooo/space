#include <iostream>
#include <tuple>
#include <set>

template <class Tup, class Func, std::size_t ...Is>
constexpr void static_for_impl(Tup&& t, Func &&f, std::index_sequence<Is...> ) {
  (f(std::integral_constant<std::size_t, Is>{}, std::get<Is>(t)),...);
}

template <class ... T, class Func >
constexpr void static_for(std::tuple<T...>&t, Func &&f) {
  static_for_impl(t, std::forward<Func>(f), std::make_index_sequence<sizeof...(T)>{});
}

int main()
{
    auto t = std::make_tuple( -1, 3, 0, 4 );

    int min = 100;
    static_for(t, [&] (auto i, auto w) {
      //if (w < min) min = w;
      if (w < min) min = w;
      std::cout << min << " " << w << std::endl;
    });

    std::cout << min << std::endl;

    return 0;
}
