#ifndef _FILE_MANAGER_HPP_
#define _FILE_MANAGER_HPP_

#include <string>

#include <DazzleEngine.hpp>

namespace Dazzle
{
    struct FileLoader
    {
        static std::string ReadFile(const std::string& path);
    };
}

#endif // _FILE_MANAGER_HPP_