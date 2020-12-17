#pragma once

#include <string>
#include <vector>

#include "raylib.h"

namespace ResourceManager
{
    void Setup(const std::string& rootPath);
    void Cleanup();

    size_t GetAssetID(const std::string& path);
    const std::string& GetAssetName(size_t id);

    std::string GetAssetPath(const std::string& path);

    const Texture2D& GetTexture(const std::string& path);
    const Texture2D& GetTexture(size_t id);

    const Image& GetImage(const std::string& path);
    const Image& GetImage(size_t id);

    void Unload(const std::string& path);
    void Unload(size_t id);

    std::vector<size_t> GetAssetIDs(const std::string& path);
}