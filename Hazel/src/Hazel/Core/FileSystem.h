#pragma once

#include <filesystem>

namespace Hazel
{

class FileSystem
{
public:
    static std::filesystem::path ResolvePath(const std::filesystem::path& path);
};

} // namespace Hazel
