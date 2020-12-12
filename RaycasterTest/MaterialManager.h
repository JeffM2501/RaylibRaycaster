#pragma once

#include <string>
#include "raylib.h"

namespace MaterialManager
{
	void Setup();
	void Cleanup();

	size_t LoadTextureMaterial(const std::string& texturePath, const Color& tint);
	size_t LoadMaterial(const std::string& materialPath);

	const Material& GetRuntimeMaterial(size_t id);

	void Unload(size_t id);
}