#include "hzpch.h"
#include "FileSystem.h"

#include <system_error>

#if defined(HZ_PLATFORM_MACOS)
#include <mach-o/dyld.h>
#elif defined(HZ_PLATFORM_WINDOWS)
#include <Windows.h>
#endif

namespace Hazel
{
namespace fs = std::filesystem;

static bool PathExists(const fs::path& path)
{
    std::error_code ec;
    return fs::exists(path, ec);
}

static fs::path MakeAbsolute(const fs::path& path)
{
    std::error_code ec;
    fs::path absolutePath = fs::absolute(path, ec);
    if (ec)
        return path.lexically_normal();

    return absolutePath.lexically_normal();
}

static fs::path GetExecutableDirectory()
{
#if defined(HZ_PLATFORM_MACOS)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);

    std::vector<char> buffer(size);
    if (_NSGetExecutablePath(buffer.data(), &size) != 0)
        return {};

    return fs::path(buffer.data()).parent_path().lexically_normal();
#elif defined(HZ_PLATFORM_WINDOWS)
    std::vector<wchar_t> buffer(MAX_PATH);
    DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0)
        return {};

    while (length == buffer.size())
    {
        buffer.resize(buffer.size() * 2);
        length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0)
            return {};
    }

    return fs::path(buffer.data(), buffer.data() + length).parent_path().lexically_normal();
#else
    return {};
#endif
}

static void AddCandidate(std::vector<fs::path>& candidates, const fs::path& candidate)
{
    if (candidate.empty())
        return;

    const fs::path normalized = candidate.lexically_normal();
    if (std::find(candidates.begin(), candidates.end(), normalized) == candidates.end())
        candidates.push_back(normalized);
}

static std::vector<fs::path> BuildCandidates(const fs::path& path)
{
    std::vector<fs::path> candidates;
    AddCandidate(candidates, path);

    if (!path.is_relative())
        return candidates;

    std::error_code ec;
    const fs::path currentPath = fs::current_path(ec);
    if (!ec)
    {
        AddCandidate(candidates, currentPath / path);
        AddCandidate(candidates, currentPath / "Sandbox" / path);
    }

    const fs::path executableDirectory = GetExecutableDirectory();
    if (!executableDirectory.empty())
    {
        AddCandidate(candidates, executableDirectory / path);

        fs::path workspaceRoot = executableDirectory;
        for (int i = 0; i < 3 && !workspaceRoot.empty(); i++)
            workspaceRoot = workspaceRoot.parent_path();

        if (!workspaceRoot.empty())
            AddCandidate(candidates, workspaceRoot / "Sandbox" / path);
    }

    return candidates;
}

std::filesystem::path FileSystem::ResolvePath(const std::filesystem::path& path)
{
    for (const auto& candidate : BuildCandidates(path))
    {
        if (PathExists(candidate))
            return MakeAbsolute(candidate);
    }

    return path;
}

} // namespace Hazel
