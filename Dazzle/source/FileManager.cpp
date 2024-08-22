#include <stdexcept>
#include <fstream>
#include <sstream>

#include "FileManager.hpp"
#include "Utilities.hpp"

std::string Dazzle::FileManager::ReadFile(const std::string& path)
{
    std::ifstream inputStream(path);

    if (inputStream.fail())
        assert_with_message(Utilities::kAlwaysFail, "Unable to open file: " + path);

    std::istreambuf_iterator<char> startIt(inputStream), endIt;
    std::string buffer(startIt, endIt);
    inputStream.close();
    return buffer;
}

std::vector<char> Dazzle::FileManager::ReadBinary(const std::string& path)
{
    std::ifstream inputStream(path, std::ios::binary);

    if (inputStream.fail())
        assert_with_message(Utilities::kAlwaysFail, "Unable to open file: " + path);

    std::istreambuf_iterator<char> startIt(inputStream), endIt;
    std::vector<char> buffer(startIt, endIt);
    inputStream.close();
    return buffer;
}

void Dazzle::FileManager::WriteFile(const std::string& path, const std::string& data)
{
    std::ofstream outputStream(path);

    if (outputStream.fail())
        assert_with_message(Utilities::kAlwaysFail, "Unable to open file: " + path);

    outputStream.write(data.c_str(), data.size());
    outputStream.close();
}

void Dazzle::FileManager::WriteBinary(const std::string& path, const char* data, const size_t length)
{
    std::ofstream outputStream(path, std::ios::binary | std::ios::trunc);

    if (outputStream.fail())
        assert_with_message(Utilities::kAlwaysFail, "Unable to open file: " + path);

    outputStream.write(data, length);
    outputStream.close();
}
