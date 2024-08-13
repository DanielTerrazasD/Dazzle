#include <stdexcept>
#include <fstream>
#include <sstream>

#include "FileManager.hpp"

std::string Dazzle::FileLoader::ReadFile(const std::string& path)
{
    std::fstream file(path, std::ios::in);

    if (!file.is_open())
    {
        std::ostringstream message;
        message << "Could not open file:\n" << path;
        throw std::runtime_error(message.str());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}