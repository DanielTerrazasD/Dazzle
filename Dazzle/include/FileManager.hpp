#ifndef _FILE_MANAGER_HPP_
#define _FILE_MANAGER_HPP_

#include <string>

#include <DazzleEngine.hpp>

namespace Dazzle
{
    struct FileManager
    {
        static std::string ReadFileFrom(const std::string& path);
        static void WriteFileTo(const std::string& path, const std::string& data);
        static void WriteBinaryTo(const std::string& path, const char* data, const size_t length);
    };
}

#endif // _FILE_MANAGER_HPP_