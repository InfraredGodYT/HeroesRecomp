// Credit to extract-xiso devs (in@fishtank.com and other extract-xiso contributors) for a lot of the implementation here
// https://github.com/XboxDev/extract-xiso/

#pragma once

#include "virtual_file_system.h"

// xiso and redump images
struct XisoFileSystem : VirtualFileSystem
{
    FilePtr stream;

    bool Read(const File& file, std::span<uint8_t> buffer, const ChunkCallback& onChunk) const override;

    static std::unique_ptr<VirtualFileSystem> Create(const std::filesystem::path& isoPath);
};
