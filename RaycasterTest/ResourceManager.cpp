#include "ResourceManager.h"
#include <map>

namespace ResourceManager
{
    std::string RootPath;

    std::map<size_t, std::string> AssetIDFilePaths;

    std::map<size_t, Texture2D> TextureList;
    std::map<size_t, Image> ImageList;


    size_t HashPath(const std::string& path)
    {
        return std::hash<std::string>{}(path);
    }

    void Setup(const std::string& rootPath)
    {
        RootPath = rootPath;
    }

    void Cleanup()
    {
        for (auto itr : TextureList)
            UnloadTexture(itr.second);
        for (auto itr : ImageList)
            UnloadImage(itr.second);

        TextureList.clear();
        ImageList.clear();
    }

    void Unload(const std::string& path)
    {
        Unload(GetAssetID(path));
    }

    void Unload(size_t id)
    {
        auto tItr = TextureList.find(id);
        if (tItr != TextureList.end())
            UnloadTexture(tItr->second);

        auto iItr = ImageList.find(id);
        if (iItr != ImageList.end())
            UnloadImage(iItr->second);
    }

    std::string GetOSPath(const std::string& path)
    {
        return RootPath + path;
    }

    size_t GetAssetID(const std::string& path)
    {
        size_t h = HashPath(path);
        if (AssetIDFilePaths.find(h) == AssetIDFilePaths.end())
        {
            AssetIDFilePaths[h] = GetOSPath(path);
        }
        return h;
    }

    const Texture2D& GetTexture(const std::string& path)
    {
        return GetTexture(GetAssetID(path));
    }

    const Texture2D& GetTexture(size_t id)
    {
        const std::string& path = AssetIDFilePaths[id];

        auto itr = TextureList.find(id);
        if (itr == TextureList.end())
        {
            TextureList.emplace(id,LoadTexture(path.c_str()));
            return TextureList[id];
        }

        return itr->second;
    }

    const Image& GetImage(const std::string& path)
    {
        return GetImage(GetAssetID(path));
    }

    const Image& GetImage(size_t id)
    {
        const std::string& path = AssetIDFilePaths[id];

        auto itr = ImageList.find(id);
        if (itr == ImageList.end())
        {
            ImageList.emplace(id,LoadImage(path.c_str()));
            return ImageList[id];
        }

        return itr->second;
    }
}