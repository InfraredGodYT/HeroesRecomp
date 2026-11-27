#include "directory_file_system.h"

bool DirectoryFileSystem::Read(const File& file, std::span<uint8_t> buffer, const ChunkCallback& onChunk) const
{
    FilePtr stream = OpenFile(file.path);
    return stream != nullptr && ReadChunks(stream.get(), file.size, buffer, onChunk);
}

std::unique_ptr<VirtualFileSystem> DirectoryFileSystem::Create(const std::filesystem::path& root)
{
    auto fileSystem = std::make_unique<DirectoryFileSystem>();
    std::error_code ec;
    std::filesystem::recursive_directory_iterator it(root, std::filesystem::directory_options::skip_permission_denied, ec);
    for (; !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec))
    {
        if (!it->is_regular_file(ec))
        {
            continue;
        }

        uint64_t size = it->file_size(ec);
        if (ec)
        {
            return nullptr;
        }

        std::u8string relativePath = it->path().lexically_relative(root).generic_u8string();
        std::string key = ToLower(std::string(relativePath.begin(), relativePath.end()));
        fileSystem->files[key] = { it->path(), 0, size };
    }

    if (ec)
    {
        return nullptr;
    }
    return fileSystem;
}
