// Credit to extract-xiso devs (in@fishtank.com and other extract-xiso contributors) for a lot of the implementation here
// https://github.com/XboxDev/extract-xiso/

#include "xiso_file_system.h"

#include <cstring>
#include <unordered_set>
#include <vector>

static constexpr uint32_t SECTOR_SIZE = 2048;
static constexpr uint32_t HEADER_OFFSET = 0x10000;
static constexpr uint32_t DWORD_SIZE = 4;
static constexpr uint32_t ENTRY_HEADER_SIZE = 14;

static constexpr uint64_t XGD1_LSEEK_OFFSET = 0x18300000u;
static constexpr uint64_t GLOBAL_LSEEK_OFFSET = 0x0FD90000u;
static constexpr uint64_t XGD3_LSEEK_OFFSET = 0x02080000u;

static constexpr const char* HEADER_MAGIC = "MICROSOFT*XBOX*MEDIA";
static constexpr uint32_t HEADER_MAGIC_LENGTH = 20;

static constexpr uint8_t ATTRIBUTE_DIR = 0x10;
static constexpr uint16_t PAD_SHORT = 0xFFFF;

static bool Seek(FILE* stream, uint64_t offset)
{
#ifdef _WIN32
    return _fseeki64(stream, int64_t(offset), SEEK_SET) == 0;
#else
    return fseeko(stream, off_t(offset), SEEK_SET) == 0;
#endif
}

static bool ReadAt(FILE* stream, uint64_t offset, void* buffer, size_t size)
{
    return Seek(stream, offset) && fread(buffer, 1, size, stream) == size;
}

static bool FindHeader(FILE* stream, uint64_t fileSize, uint64_t& discOffset)
{
    char magic[HEADER_MAGIC_LENGTH];

    auto tryOffset = [&](uint64_t offset)
    {
        if (!ReadAt(stream, HEADER_OFFSET + offset, magic, HEADER_MAGIC_LENGTH) || memcmp(magic, HEADER_MAGIC, HEADER_MAGIC_LENGTH) != 0)
        {
            return false;
        }

        discOffset = offset;
        return true;
    };

    if (tryOffset(0) || tryOffset(XGD1_LSEEK_OFFSET) || tryOffset(GLOBAL_LSEEK_OFFSET) || tryOffset(XGD3_LSEEK_OFFSET))
    {
        return true;
    }

    for (uint64_t offset = 0x10000; offset + HEADER_OFFSET < fileSize; offset += 0x10000)
    {
        if (tryOffset(offset))
        {
            return true;
        }
    }
    return false;
}

static bool ParseDirectory(FILE* stream, uint64_t imageSize, uint64_t discOffset, uint32_t sector, uint32_t size, const std::string& prefix,
    std::unordered_map<std::string, VirtualFileSystem::File>& files, uint32_t depth)
{
    if (size == 0)
    {
        return true;
    }

    // corrupt images can loop
    if (depth > 64)
    {
        return false;
    }

    uint64_t tableOffset = uint64_t(sector) * SECTOR_SIZE + discOffset;
    if (tableOffset + size > imageSize)
    {
        return false;
    }

    std::vector<uint8_t> table(size);
    if (!ReadAt(stream, tableOffset, table.data(), size))
    {
        return false;
    }

    std::vector<uint32_t> pending{ 0 };
    std::unordered_set<uint32_t> visited;

    while (!pending.empty())
    {
        uint32_t offset = pending.back();
        pending.pop_back();

        if (offset + ENTRY_HEADER_SIZE > size || !visited.insert(offset).second)
        {
            continue;
        }

        uint16_t left, right;
        uint32_t startSector, fileSize;
        memcpy(&left, &table[offset], 2);
        memcpy(&right, &table[offset + 2], 2);
        memcpy(&startSector, &table[offset + 4], 4);
        memcpy(&fileSize, &table[offset + 8], 4);
        uint8_t attributes = table[offset + 12];
        uint8_t nameLength = table[offset + 13];

        if (left == PAD_SHORT)
        {
            continue;
        }

        if (offset + ENTRY_HEADER_SIZE + nameLength > size)
        {
            return false;
        }

        if (left)
        {
            pending.push_back(uint32_t(left) * DWORD_SIZE);
        }

        if (right)
        {
            pending.push_back(uint32_t(right) * DWORD_SIZE);
        }

        std::string name(reinterpret_cast<const char*>(&table[offset + ENTRY_HEADER_SIZE]), nameLength);
        if (name.empty() || name == "." || name == ".." || name.find_first_of("/\\") != std::string::npos)
        {
            continue;
        }

        std::string path = prefix + name;
        if (attributes & ATTRIBUTE_DIR)
        {
            if (!ParseDirectory(stream, imageSize, discOffset, startSector, fileSize, path + "/", files, depth + 1))
            {
                return false;
            }
        }
        else
        {
            files[VirtualFileSystem::ToLower(path)] = { {}, uint64_t(startSector) * SECTOR_SIZE + discOffset, fileSize };
        }
    }
    return true;
}

bool XisoFileSystem::Read(const File& file, std::span<uint8_t> buffer, const ChunkCallback& onChunk) const
{
    return Seek(stream.get(), file.offset) && ReadChunks(stream.get(), file.size, buffer, onChunk);
}

std::unique_ptr<VirtualFileSystem> XisoFileSystem::Create(const std::filesystem::path& isoPath)
{
    std::error_code ec;
    uint64_t fileSize = std::filesystem::file_size(isoPath, ec);

    // cap at 10GB, just as a sanity check
    if (ec || fileSize > 10ull * 1024 * 1024 * 1024)
    {
        return nullptr;
    }

    auto fileSystem = std::make_unique<XisoFileSystem>();
    fileSystem->stream = OpenFile(isoPath);
    if (fileSystem->stream == nullptr)
    {
        return nullptr;
    }

    uint64_t discOffset = 0;
    if (!FindHeader(fileSystem->stream.get(), fileSize, discOffset))
    {
        return nullptr;
    }

    uint32_t root[2];
    if (!ReadAt(fileSystem->stream.get(), HEADER_OFFSET + discOffset + HEADER_MAGIC_LENGTH, root, sizeof(root)) || (root[0] == 0 && root[1] == 0))
    {
        return nullptr;
    }

    if (!ParseDirectory(fileSystem->stream.get(), fileSize, discOffset, root[0], root[1], "", fileSystem->files, 0))
    {
        return nullptr;
    }
    return fileSystem;
}
