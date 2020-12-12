#include "MaterialManager.h"
#include "ResourceManager.h"
#include <map>
#include <memory>

namespace MaterialManager
{
	class MaterialInfo
	{
	public:
		size_t ID;

		Material EngineMaterial;

		virtual void LoadMaterial() = 0;
	};

	class TextureMaterial : public MaterialInfo
	{
	public:
		size_t TextureID;
		Color Tint;

		void LoadMaterial() override
		{
			EngineMaterial = LoadMaterialDefault();
			EngineMaterial.maps[MAP_DIFFUSE].texture = ResourceManager::GetTexture(TextureID);
			EngineMaterial.maps[MAP_DIFFUSE].color = Tint;
		}
	};

	std::map<size_t, std::shared_ptr<MaterialInfo>> MaterialCache;

	Material DefaultMaterial = LoadMaterialDefault();

	void Setup()
	{
		Image emptyImage = GenImageChecked(128, 128, 4, 4, RAYWHITE, GRAY);
		DefaultMaterial.maps[MAP_DIFFUSE].texture = LoadTextureFromImage(emptyImage);
		DefaultMaterial.maps[MAP_DIFFUSE].color = WHITE;

		UnloadImage(emptyImage);
	}

	void Cleanup()
	{
		UnloadMaterial(DefaultMaterial);

		for (auto& m : MaterialCache)
			UnloadMaterial(m.second->EngineMaterial);

		MaterialCache.clear();
	}

	size_t GetHash(const std::string& path)
	{
		return std::hash<std::string>{}(path);
	}

	size_t LoadTextureMaterial(const std::string& texturePath, const Color& tint)
	{
		size_t hash = GetHash(texturePath + TextFormat("%d", tint));

		if (MaterialCache.find(hash) != MaterialCache.end())
			return hash;

		std::shared_ptr<TextureMaterial> tx = std::make_shared<TextureMaterial>();
		tx->ID = hash;
		tx->TextureID = ResourceManager::GetAssetID(texturePath);
		tx->Tint = tint;
		tx->LoadMaterial();

		MaterialCache.emplace(hash, tx);
		return hash;
	}

	size_t LoadMaterial(const std::string& materialPath)
	{
		// TODO, add other material types
		return LoadTextureMaterial(materialPath, WHITE);
	}

	const Material& GetRuntimeMaterial(size_t id)
	{
		std::map<size_t, std::shared_ptr<MaterialInfo>>::iterator itr = MaterialCache.find(id);

		if (itr == MaterialCache.end() || itr->second == nullptr)
			return DefaultMaterial;

		return itr->second->EngineMaterial;
	}
}
