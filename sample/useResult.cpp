#include <iostream>
#include "Result.h"
#include "ToString.h"

auto main() -> int {

    using sls::Result; // declared in namespace sls
    using sls::ToString;

    std::cout << "Examples on usage of Result<T>\n";

    // Result exposes the underlying constructors of std::vector
    Result<int> res{1, 2, 3, 4, 5};
    std::cout << "res: " << res << '\n';

    Result<double> res2(5, 3.7);
    std::cout << "res2: " << res2 << '\n';

    // and can be converted to and from a vector. However, the
    // conversion to a vector is not efficient since a copy is made
    // and should only be done when a vector is needed for further use
    // in most sense and in standard algorithms Result<T> behaves as a
    // vector.
    std::vector<int> vec(5, 5);
    std::cout << "vec: " << ToString(vec) << "\n";

    Result<int> res3 = vec;
    std::cout << "res3: " << res3 << '\n';

    std::vector<int> vec2 = res3;
    std::cout << "vec2: " << ToString(vec2) << "\n";


    // WARNING this feature is still not decied, its convenient 
    // but might turn out to be a source of bugs...
    // it is also possible to convert from Result<T> to T
    int r = res3;
    std::cout << "r: " << r << '\n';
    std::cout << "static_cast<int>: " << static_cast<int>(res3) << '\n';
    std::cout << "static_cast<double>: " << static_cast<double>(res3) << '\n';

    int r2 = res;
    std::cout << "res: " << res << " converted to int: " << r2 << "\n";

    //Using squash we can also convert to a single value
    std::cout << "res.squash(): " << res.squash() << '\n';
    std::cout << "res3.squash(): " << res3.squash() << '\n';


    //.squash also takes a default value
    std::cout << "res.squash(-1): " << res.squash(-1) << '\n';
    std::cout << "res3.squash(-1): " << res3.squash(-1) << '\n';

    //
}