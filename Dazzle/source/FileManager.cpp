#include <stdexcept>
#include <fstream>
#include <sstream>

#include "FileManager.hpp"

std::string Dazzle::FileManager::ReadFileFrom(const std::string& path)
{
    std::fstream file(path, std::ios::in);

    if (!file.is_open())
    {
        std::ostringstream message;
        message << "Could not open file:\n" << path << '\n';
        throw std::runtime_error(message.str());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Dazzle::FileManager::WriteFileTo(const std::string& path, const std::string& data)
{
    std::ofstream file(path.c_str());

    if (!file.is_open())
    {
        std::ostringstream message;
        message << "Could not open file:\n" << path << '\n';
        throw std::runtime_error(message.str());
    }

    file.write(data.c_str(), data.size());
    file.close();
}

void Dazzle::FileManager::WriteBinaryTo(const std::string& path, const char* data, const size_t length)
{
    std::ofstream file(path.c_str(), std::ios::binary | std::ios::trunc);

    if (!file.is_open())
    {
        std::ostringstream message;
        message << "Could not open file:\n" << path << '\n';
        throw std::runtime_error(message.str());
    }

    file.write(data, length);
    file.close();
}
