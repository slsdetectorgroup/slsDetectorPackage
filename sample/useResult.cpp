// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/** Examples on how to use Result<T> */

#include "sls/Result.h"
#include "sls/ToString.h"
#include <algorithm>
#include <iostream>

using sls::Result;
using sls::ToString;

auto main() -> int {

    std::cout << "Examples on usage of Result<T>\n";

    /** Constructing Result<T> can be done in the same way as vectors */
    Result<int> res{1, 2, 3, 4, 5}; 
    std::cout << "res: " << res << '\n';

    Result<double> res2(5, 3.7);
    std::cout << "res2: " << res2 << '\n';

    /** and Result can be converted to and from a vector. However, the
     * conversion to a vector is not efficient since a copy is made
     * and should only be done when a vector is needed for further use
     * in most cases Result behaved as a vector, for example with standard
     *  algorithms */


    std::vector<int> vec(5, 5);
    std::cout << "vec: " << ToString(vec) << "\n";

    //Result from vector
    Result<int> res3 = vec;
    std::cout << "res3: " << res3 << '\n';

    // Vector from Result
    std::vector<int> vec2 = res3;
    std::cout << "vec2: " << ToString(vec2) << "\n";

    // Using squash we can convert to a single value
    std::cout << "res.squash(): " << res.squash() << '\n';
    std::cout << "res3.squash(): " << res3.squash() << '\n';

    //.squash also takes a default value
    std::cout << "res.squash(-1): " << res.squash(-1) << '\n';
    std::cout << "res3.squash(-1): " << res3.squash(-1) << '\n';

    Result<int> ivec{1, 3, 5};

    Result<sls::time::ns> nres(ivec);
    std::cout << "nres: " << sls::ToString(nres) << '\n';

    /* Convert from Result<int> to Result<bool> */
    Result<int> int_result{0, 1, 0, 3, -5};
    Result<bool> bool_result{int_result};
    std::cout << bool_result << '\n';

    // Result can be printed using <<
    Result<std::string> string_res{"ein", "zwei", "drei"};
    std::cout << string_res << '\n';
}