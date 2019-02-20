#pragma once

#include <string>
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
    strncpy(destination, source, array_size-1);
    destination[array_size - 1] = '\0';
}

/*
Removes all occurrences of the specified char from a c string
Templated on array size to ensure no access after buffer limits.
*/
template <size_t array_size>
void removeChar(char (&str)[array_size], char ch) {
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

/*
Concatenate the non empty strings in the vector using +
*/
std::string concatenateNonEmptyStrings(const std::vector<std::string> &vec);

/*
Concatenate strings using + if the strings are different
*/
std::string concatenateIfDifferent(const std::vector<std::string> &container);

/*
Convert an ip address string to a string in hex format. (removing dots)
*/
std::string stringIpToHex(const std::string &ip);

}; // namespace sls
