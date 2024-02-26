// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace sls {

/* Implementation of a safe string copy function for setting fields in
for example the multi sls detector. It tries to copy the size of the
destination from the source, stopping on '\0'.

Warning this will truncate the source string and should be used with care.
Still this is better than strcpy and a buffer overflow...
*/
template <size_t array_size>
void strcpy_safe(char (&destination)[array_size], const char *source) {
    assert(source != nullptr);
    assert(array_size > strlen(source));
    strncpy(destination, source, array_size - 1);
    destination[array_size - 1] = '\0';
}

template <size_t array_size>
void strcpy_safe(char (&destination)[array_size], const std::string &source) {
    assert(array_size > source.size());
    strncpy(destination, source.c_str(), array_size - 1);
    destination[array_size - 1] = '\0';
}

/*
Removes all occurrences of the specified char from a c string
Templated on array size to ensure no access after buffer limits.
*/
template <size_t array_size> void removeChar(char (&str)[array_size], char ch) {
    int count = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] != ch)
            str[count++] = str[i];
        if (i == array_size - 1)
            break;
    }
    str[count] = '\0';
}

/*
Split a string using the specified delimeter and return a vector of strings.
TODO! Look into switching to absl or a string_view based implementation. Current
implementation should not be used in a performance critical place.
*/
std::vector<std::string> split(const std::string &strToSplit, char delimeter);

std::string RemoveUnit(std::string &str);

bool is_int(const std::string &s);

bool replace_first(std::string *s, const std::string &substr,
                   const std::string &repl);

std::pair<std::string, uint16_t> ParseHostPort(const std::string &s);

} // namespace sls
