#pragma once

//#include "sls/StaticVector.h"
#include "sls/string_utils.h"

#include <optional>
#include <string>
#include <stdexcept>
#include <map>
#include <algorithm>

namespace sls {

template <size_t N>
struct FixedString {
    char data_[N]{};
    constexpr FixedString() noexcept { memset(data_, 0, N); }
    FixedString(const char (&s)[N]) {
        if (N <= 1) {
            throw std::runtime_error("FixedString cannot be empty");
        }
        strcpy_checked<N>(data_, s);
    }
    FixedString(const std::string& s) {
        if (s.size() <= 1) {
            throw std::runtime_error("FixedString cannot be empty");
        }
        strcpy_checked<N>(data_, s);
    }
    bool operator==(const FixedString& other) const noexcept {
        return std::strncmp(data_, other.data_, N) == 0;
    }
    bool operator<(const FixedString& other) const noexcept {
        return std::strncmp(data_, other.data_, N) < 0;
    }
    std::string str() const {
        return std::string(data_);
    }
};

template <typename Key, typename Value, size_t Capacity, bool Unique_Values>

class MapOnStack {
    // for shared memory use only trivially copyable types
    static_assert(std::is_trivially_copyable_v<Key>);
    static_assert(std::is_trivially_copyable_v<Value>);
    static_assert(std::is_standard_layout_v<Key>);
    static_assert(std::is_standard_layout_v<Value>);
    static_assert(!std::is_pointer_v<Key>);
    static_assert(!std::is_pointer_v<Value>);

    public:

    constexpr MapOnStack() noexcept = default;

    constexpr size_t size() const noexcept { return current_size_; }
    constexpr size_t capacity() const noexcept { return Capacity; }
    constexpr bool empty() const noexcept { return current_size_ == 0; }
    void clear() noexcept {
        current_size_ = 0;
    }
   
    bool containsKey(const Key& key) const {
        return lookupEntryByKey(key).has_value();
    }

    bool hasValue(const Value& value) const {
        return lookupEntryByValue(value).has_value();
    }

    void addKeyOrSetValue(const Key& key, const Value& value) {
        if (auto entry = findEntryByKey(key)) {
            (*entry)->setValue(value);
            return;
        }
        addEntry(key, value);
    }

    void addKey(const Key& key, const Value& value) {
        addEntry(key, value);
    }

    void setValue(const Key& key, const Value& value) {
        if (auto entry = findEntryByKey(key)) {
            (*entry)->setValue(value);
            return;
        }
        throw std::runtime_error("Key not found. Cannot set value.");
    }   

    Value getValue(const Key& key) const {
        auto val = lookupEntryByKey(key);
        if (!val.has_value()) {
            throw std::runtime_error("No entry found for key");
        }
        return val.value();
    }

    Key getKey(const Value& value) const {
        auto key = lookupEntryByValue(value);
        if (!key.has_value()) {
            throw std::runtime_error("No entry found for value");
        }
        return key.value();
    }

    void setMap(const std::map<Key, Value> &list) {
        if (list.size() >= Capacity) {
            throw std::runtime_error("List size exceeds maximum Capacity");
        }
        clear();
        for (const auto &[key, value] : list) {
            addEntry(key, value);
        }
    }

    std::map<Key, Value> getMap() const {
        std::map<Key, Value> list;
        for (size_t i = 0; i != current_size_; ++i)
            list[data_[i].key()] = data_[i].value();
        return list;
    }

  private:
    struct Entry {
        Key key_{};
        Value value_{};
        constexpr Entry() noexcept = default;
        constexpr Entry(const Key& key, const Value& value) : key_(key),value_(value) {}
        constexpr Key key() const noexcept { return key_; }
        constexpr Value value() const noexcept { return value_; }
        constexpr void setValue(Value value) noexcept { value_ = value; }
    };


    Entry data_[Capacity];
    size_t current_size_{0};
    //StaticVector<Entry, Capacity> entries_;

    std::optional<Entry*> findEntryByKey(const Key& key) {
        auto it = std::find_if(data_, data_ + current_size_,
                               [&key](Entry &e) { 
                                return (e.key() == key); });
        if (it != data_ + current_size_)
            return it;
        return std::nullopt;
    }

    std::optional<const Entry*> findEntryByKey(const Key& key) const {
        auto it = std::find_if(data_, data_ + current_size_,
                               [&key](const Entry &e) { 
                                return (e.key() == key); });
        if (it != data_ + current_size_)
            return it;
        return std::nullopt;
    }

    std::optional<Entry *> findEntryByValue(Value value) {
        if (!Unique_Values) {
            throw std::runtime_error(
                "Cannot lookup by value when unique values are not enforced.");
        }
        auto it = std::find_if(data_, data_ + current_size_,
                               [&value](Entry &e) { 
                                return e.value() == value; });
        if (it != data_ + current_size_)
            return it;
        return std::nullopt;
    }

    std::optional<const Entry *> findEntryByValue(Value value) const {
        if (!Unique_Values) {
            throw std::runtime_error(
                "Cannot lookup by value when unique values are not enforced.");
        }
        auto it = std::find_if(data_, data_ + current_size_,
                               [&value](const Entry &e) { 
                                return e.value() == value; });
        if (it != data_ + current_size_)
            return it;
        return std::nullopt;
    }

    std::optional<const Value> lookupEntryByKey(const Key& key) const {
        auto entry = findEntryByKey(key);
        return (entry ? std::optional<Value>((*entry)->value()) : std::nullopt);
    }

    std::optional<Key> lookupEntryByValue(Value value) const {
        auto entry = findEntryByValue(value);
        return (entry ? std::optional<Key>((*entry)->key())
                      : std::nullopt);
    }

    void checkDuplicateKey(const Key& key) const {
        if (auto entry = findEntryByKey(key)) {
            throw std::runtime_error("Key already exists. Cannot have duplicate keys.");
        }
    }

    void checkDuplicateValue(Value value) const {
        if (Unique_Values) {
            if (auto entry = findEntryByValue(value)) {
                throw std::runtime_error("Value already assigned to another key '" + (*entry)->key().str() + "'. Cannot assign it again.");
            }
        }
    }

    void checkSize() const {
        if (current_size_ >= Capacity) {
            throw std::runtime_error("Maximum capacity reached");
        }
    }

    void addEntry(const Key& key, const Value& value) {
        checkSize();
        checkDuplicateKey(key);
        checkDuplicateValue(value);

        data_[current_size_] = Entry(key, value);
        ++(current_size_);
    }
};

} // namespace sls
