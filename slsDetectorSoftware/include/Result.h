#pragma once
#include <vector>
#include <algorithm>
namespace sls {

template <typename Container>
typename Container::value_type
Squash(const Container &c, typename Container::value_type default_value = {}) {
    if (!c.empty() && std::all_of(begin(c), end(c),
                    [c](const typename Container::value_type &element) {
                        return element == c.front();
                    }))
        return c.front();
    return default_value;
}

template <class T, class Allocator = std::allocator<T>> class Result {
    std::vector<T, Allocator> vec;

  public:
    Result() = default;
    Result(std::initializer_list<T> list) : vec(list){};
    template <typename... Args>
    Result(Args &&... args) : vec(std::forward<Args>(args)...) {}
    using value_type = T;
    using iterator = decltype(vec.begin());
    using const_iterator = decltype(vec.cbegin());
    using size_type = decltype(vec.size());

    auto begin() -> decltype(vec.begin()) { return vec.begin(); }
    auto end() -> decltype(vec.end()) { return vec.end(); }
    auto cbegin() const -> decltype(vec.cbegin()) { return vec.cbegin(); }
    auto cend() const -> decltype(vec.cend()) { return vec.cend(); }
    auto size() const -> decltype(vec.size()) { return vec.size(); }
    auto empty() const -> decltype(vec.empty()) { return vec.empty(); }

    T squash(){
        return Squash(vec);
    }
};

} // namespace sls