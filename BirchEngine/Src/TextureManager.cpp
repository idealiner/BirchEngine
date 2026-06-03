#include "TextureManager.h"

#include "SDL_filesystem.h"

#include <cstdio>
#include <string>
#include <vector>

static std::string JoinPath(const std::string& base, const std::string& rel)
{
	if (base.empty())
	{
		return rel;
	}

	if (base.back() == '/' || base.back() == '\\')
	{
		return base + rel;
	}

	return base + "/" + rel;
}

static std::string BaseName(const std::string& path)
{
	size_t pos = path.find_last_of("/\\");
	if (pos == std::string::npos)
	{
		return path;
	}

	return path.substr(pos + 1);
}

SDL_Texture* TextureManager::LoadTexture(const char* texture, SDL_Renderer* ren)
{
	if (!texture || !ren)
	{
		return nullptr;
	}

	std::string requested(texture);
	std::string fileOnly = BaseName(requested);

	std::vector<std::string> candidates;
	candidates.push_back(requested);
	candidates.push_back(JoinPath("assets", fileOnly));
	candidates.push_back(JoinPath("../assets", fileOnly));
	candidates.push_back(JoinPath("../../assets", fileOnly));
	candidates.push_back(JoinPath("BirchEngine/assets", fileOnly));

	char* basePathRaw = SDL_GetBasePath();
	if (basePathRaw)
	{
		std::string basePath(basePathRaw);
		SDL_free(basePathRaw);

		candidates.push_back(JoinPath(basePath, requested));
		candidates.push_back(JoinPath(basePath, JoinPath("assets", fileOnly)));
		candidates.push_back(JoinPath(basePath, JoinPath("../assets", fileOnly)));
		candidates.push_back(JoinPath(basePath, JoinPath("../../assets", fileOnly)));
		candidates.push_back(JoinPath(basePath, JoinPath("../BirchEngine/assets", fileOnly)));
	}

	SDL_Surface* tempSurface = nullptr;
	for (const std::string& path : candidates)
	{
		tempSurface = IMG_Load(path.c_str());
		if (tempSurface)
		{
			break;
		}
	}

	if (!tempSurface)
	{
		std::fprintf(stderr, "TextureManager: failed to load '%s' (%s)\n", texture, IMG_GetError());
		return nullptr;
	}

	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, tempSurface);
	SDL_FreeSurface(tempSurface);

	return tex;
}

SDL_Surface* TextureManager::LoadSurface(const char* texture)
{
	if (!texture)
	{
		return nullptr;
	}

	std::string requested(texture);
	std::string fileOnly = BaseName(requested);

	std::vector<std::string> candidates;
	candidates.push_back(requested);
	candidates.push_back(JoinPath("assets", fileOnly));
	candidates.push_back(JoinPath("../assets", fileOnly));
	candidates.push_back(JoinPath("../../assets", fileOnly));
	candidates.push_back(JoinPath("BirchEngine/assets", fileOnly));

	char* basePathRaw = SDL_GetBasePath();
	if (basePathRaw)
	{
		std::string basePath(basePathRaw);
		SDL_free(basePathRaw);

		candidates.push_back(JoinPath(basePath, requested));
		candidates.push_back(JoinPath(basePath, JoinPath("assets", fileOnly)));
		candidates.push_back(JoinPath(basePath, JoinPath("../assets", fileOnly)));
		candidates.push_back(JoinPath(basePath, JoinPath("../../assets", fileOnly)));
		candidates.push_back(JoinPath(basePath, JoinPath("../BirchEngine/assets", fileOnly)));
	}

	for (const std::string& path : candidates)
	{
		SDL_Surface* surface = IMG_Load(path.c_str());
		if (surface)
		{
			return surface;
		}
	}

	std::fprintf(stderr, "TextureManager: failed to load surface '%s' (%s)\n", texture, IMG_GetError());
	return nullptr;
}