#pragma once
#include <vector>
namespace sls {

template <class T, class Allocator = std::allocator<T>> class Result {
    std::vector<T, Allocator> vec;

  public:
    Result() = default;
    Result(std::initializer_list<T> list) : vec(list){};
    template <typename... Args>
    Result(Args &&... args) : vec(std::forward<Args>(args)...) {}
    using value_type = T;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using size_type = typename std::vector<T>::size_type;

    auto begin() -> decltype(vec.begin()) { return vec.begin(); }
    auto end() -> decltype(vec.end()) { return vec.end(); }
    auto cbegin() const -> decltype(vec.cbegin()) { return vec.cbegin(); }
    auto cend() const -> decltype(vec.cend()) { return vec.cend(); }
    auto size() const -> decltype(vec.size()) { return vec.size(); }
    auto empty() const -> decltype(vec.empty()) { return vec.empty(); }
};

} // namespace sls