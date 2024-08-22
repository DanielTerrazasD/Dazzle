#ifndef _FILE_MANAGER_HPP_
#define _FILE_MANAGER_HPP_

#include <string>
#include <vector>

#include <DazzleEngine.hpp>

namespace Dazzle
{
    struct FileManager
    {
        static std::string ReadFile(const std::string& path);
        static std::vector<char> ReadBinary(const std::string& path);
        static void WriteFile(const std::string& path, const std::string& data);
        static void WriteBinary(const std::string& path, const char* data, const size_t length);
    };
}

#endif // _FILE_MANAGER_HPP_