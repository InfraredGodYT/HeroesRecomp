#pragma once

#include "virtual_file_system.h"

struct DirectoryFileSystem : VirtualFileSystem
{
    bool Read(const File& file, std::span<uint8_t> buffer, const ChunkCallback& onChunk) const override;

    static std::unique_ptr<VirtualFileSystem> Create(const std::filesystem::path& root);
};
