#pragma once
#include <algorithm>
#include <vector>
namespace sls {

template <typename Container>
typename Container::value_type
Squash(const Container &c, typename Container::value_type default_value = {}) {
    if (!c.empty() &&
        std::all_of(begin(c), end(c),
                    [c](const typename Container::value_type &element) {
                        return element == c.front();
                    }))
        return c.front();
    return default_value;
}

template <typename Container> bool Equal(const Container &c) {
    if (!c.empty() &&
        std::all_of(begin(c), end(c),
                    [c](const typename Container::value_type &element) {
                        return element == c.front();
                    }))
        return true;
    return false;
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
    auto begin() const -> decltype(vec.begin()) { return vec.begin(); }
    auto end() const -> decltype(vec.end()) { return vec.end(); }
    auto cbegin() const -> decltype(vec.cbegin()) { return vec.cbegin(); }
    auto cend() const -> decltype(vec.cend()) { return vec.cend(); }
    auto size() const -> decltype(vec.size()) { return vec.size(); }
    auto empty() const -> decltype(vec.empty()) { return vec.empty(); }
    auto front() const -> decltype(vec.front()) { return vec.front(); }
    auto front() -> decltype(vec.front()) { return vec.front(); }
    T squash() { return Squash(vec); }

    bool equal() { return Equal(vec); }
    bool equal() const { return Equal(vec); }
};

} // namespace sls