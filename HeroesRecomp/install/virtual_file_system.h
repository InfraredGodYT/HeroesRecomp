#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>

struct VirtualFileSystem
{
    struct File
    {
        std::filesystem::path path;
        uint64_t offset;
        uint64_t size;
    };

    struct FileCloser
    {
        void operator()(FILE* file) const
        {
            fclose(file);
        }
    };

    using FilePtr = std::unique_ptr<FILE, FileCloser>;
    using ChunkCallback = std::function<bool(const uint8_t* data, size_t size)>;
    std::unordered_map<std::string, File> files;

    virtual ~VirtualFileSystem() = default;

    virtual bool Read(const File& file, std::span<uint8_t> buffer, const ChunkCallback& onChunk) const = 0;

    static std::string ToLower(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return char(std::tolower(c)); });
        return str;
    }

    const File* Find(const char* path) const
    {
        auto findResult = files.find(ToLower(path));
        return findResult != files.end() ? &findResult->second : nullptr;
    }

    bool exists(const std::string& path) const
    {
        return Find(path.c_str()) != nullptr;
    }

    uint64_t getSize(const std::string& path) const
    {
        const File* file = Find(path.c_str());
        return file != nullptr ? file->size : 0;
    }

    static FilePtr OpenFile(const std::filesystem::path& path)
    {
#ifdef _WIN32
        return FilePtr(_wfopen(path.c_str(), L"rb"));
#else
        return FilePtr(fopen(path.c_str(), "rb"));
#endif
    }

    static bool ReadChunks(FILE* file, uint64_t size, std::span<uint8_t> buffer, const ChunkCallback& onChunk)
    {
        while (size > 0)
        {
            size_t chunkSize = size_t(std::min<uint64_t>(size, buffer.size()));
            if (fread(buffer.data(), 1, chunkSize, file) != chunkSize || !onChunk(buffer.data(), chunkSize))
            {
                return false;
            }
            size -= chunkSize;
        }
        return true;
    }
};
