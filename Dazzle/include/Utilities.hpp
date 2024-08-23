#ifndef _UTILITIES_HPP_
#define _UTILITIES_HPP_

#ifdef _DEBUG
#include <iostream>
#include <cassert>

#define assert_with_message(expression, message)                                            \
    ((expression) ? static_cast<void>(0) :                                                  \
    (std::cerr  << "Assertion failed: (" << #expression << ").\n"                           \
                << "Function: " << __FUNCTION__ << '\n'                                     \
                << "File: " << __FILE__ << '\n'                                             \
                << "Line: " << __LINE__ << '\n'                                             \
                << "Message: " << message << std::endl, std::abort()))

#else
#define assert_with_message(expression, message)
#endif // _DEBUG


namespace Dazzle
{
    namespace Utilities
    {
        const bool kAlwaysFail = false;
    }
}

#endif // _UTILITIES_HPP_