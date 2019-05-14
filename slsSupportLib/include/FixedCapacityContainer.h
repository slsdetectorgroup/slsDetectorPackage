#pragma once
#include <array>
#include <cassert>
#include <stdexcept>
#include <vector>

namespace sls {
template <typename T, size_t Capacity> class FixedCapacityContainer {
  public:
    FixedCapacityContainer() = default;
    explicit FixedCapacityContainer(std::initializer_list<T> l);
    explicit FixedCapacityContainer(const std::vector<T> &v);

    template <size_t OtherCapacity>
    explicit FixedCapacityContainer(
        const FixedCapacityContainer<T, OtherCapacity> &other) noexcept;

    FixedCapacityContainer &operator=(const std::vector<T> &other);

    bool operator==(const std::vector<T> &other) const noexcept;
    bool operator!=(const std::vector<T> &other) const noexcept;

    operator std::vector<T>(){return std::vector<T>(begin(), end());}

    template <size_t OtherCapacity>
    bool operator==(const FixedCapacityContainer<T, OtherCapacity> &other) const
        noexcept;

    template <size_t OtherCapacity>
    bool operator!=(const FixedCapacityContainer<T, OtherCapacity> &other) const
        noexcept;

    T &operator[](size_t i) { return data_[i]; }
    const T &operator[](size_t i) const { return data_[i]; }

    constexpr size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    constexpr size_t capacity() const noexcept { return Capacity; }

    void push_back(const T &value);
    void resize(size_t new_size);
    void erase(T *ptr);
    T &front() noexcept { return data_.front(); }
    T &back() noexcept { return data_[size_ - 1]; }
    constexpr const T &front() const noexcept { return data_.front(); }
    constexpr const T &back() const noexcept { return data_[size_ - 1]; }

    // iterators
    T *begin() noexcept { return &data_[0]; }
    T *end() noexcept { return &data_[size_]; }
    const T *cbegin() const noexcept { return &data_[0]; }
    const T *cend() const noexcept { return &data_[size_]; }

  private:
    size_t size_{0};
    std::array<T, Capacity> data_;
} __attribute__((packed));

/* Member functions */
template <typename T, size_t Capacity>
FixedCapacityContainer<T, Capacity>::FixedCapacityContainer(
    std::initializer_list<T> l) {
    size_ = l.size();
    std::copy(l.begin(), l.end(), data_.begin());
}

template <typename T, size_t Capacity>
FixedCapacityContainer<T, Capacity>::FixedCapacityContainer(
    const std::vector<T> &v) {
    if (v.size() > Capacity) {
        throw std::runtime_error(
            "Capacity needs to be same size or larger than vector");
    }
    size_ = v.size();
    std::copy(v.begin(), v.end(), data_.begin());
}

template <typename T, size_t Capacity>
template <size_t OtherCapacity>
FixedCapacityContainer<T, Capacity>::FixedCapacityContainer(
    const FixedCapacityContainer<T, OtherCapacity> &other) noexcept {
    static_assert(Capacity >= OtherCapacity,
                  "Container needs to be same size or larger");
    size_ = other.size();
    std::copy(other.cbegin(), other.cend(), data_.begin());
}

template <typename T, size_t Capacity>
void FixedCapacityContainer<T, Capacity>::push_back(const T &value) {
    if (size_ == Capacity) {
        throw std::runtime_error("Container is full");
    } else {
        data_[size_] = value;
        ++size_;
    }
}
template <typename T, size_t Capacity>
void FixedCapacityContainer<T, Capacity>::resize(size_t new_size) {
    if (new_size > Capacity) {
        throw std::runtime_error("Cannot resize beyond capacity");
    } else {
        size_ = new_size;
    }
}

template <typename T, size_t Capacity>
FixedCapacityContainer<T, Capacity> &FixedCapacityContainer<T, Capacity>::
operator=(const std::vector<T> &other) {
    std::copy(other.begin(), other.end(), data_.begin());
    size_ = other.size();
    return *this;
}

template <typename T, size_t Capacity>
bool FixedCapacityContainer<T, Capacity>::
operator==(const std::vector<T> &other) const noexcept {
    if (size_ != other.size()) {
        return false;
    } else {
        for (size_t i = 0; i != size_; ++i) {
            if (data_[i] != other[i]) {
                return false;
            }
        }
    }
    return true;
}

template <typename T, size_t Capacity>
bool FixedCapacityContainer<T, Capacity>::
operator!=(const std::vector<T> &other) const noexcept {
    return !(*this == other);
}

template <typename T, size_t Capacity>
template <size_t OtherCapacity>
bool FixedCapacityContainer<T, Capacity>::
operator==(const FixedCapacityContainer<T, OtherCapacity> &other) const
    noexcept {
    if (size_ != other.size()) {
        return false;
    } else {
        for (size_t i = 0; i != size_; ++i) {
            if (data_[i] != other[i]) {
                return false;
            }
        }
    }
    return true;
}

template <typename T, size_t Capacity>
template <size_t OtherCapacity>
bool FixedCapacityContainer<T, Capacity>::
operator!=(const FixedCapacityContainer<T, OtherCapacity> &other) const
    noexcept {
    return !(*this == other);
}

template <typename T, size_t Capacity>
void FixedCapacityContainer<T, Capacity>::erase(T *ptr) {
    if (ptr >= begin() && ptr < end()) {
        size_ = static_cast<size_t>(ptr - begin());
    } else {
        throw std::runtime_error("tried to erase with a ptr outside obj");
    }
}

/* Free function concerning FixedCapacityContainer */
template <typename T, size_t Capacity>
constexpr T *begin(FixedCapacityContainer<T, Capacity> &container) noexcept {
    return container.begin();
}

template <typename T, size_t Capacity>
constexpr T *end(FixedCapacityContainer<T, Capacity> &container) noexcept {
    return container.end();
}

template <typename T, size_t Capacity>
bool operator==(
    const std::vector<T> &vec,
    const FixedCapacityContainer<T, Capacity> &fixed_container) noexcept {
    return fixed_container == vec;
}

template <typename T, size_t Capacity>
bool operator!=(
    const std::vector<T> &vec,
    const FixedCapacityContainer<T, Capacity> &fixed_container) noexcept {
    return fixed_container != vec;
}

} // namespace sls
