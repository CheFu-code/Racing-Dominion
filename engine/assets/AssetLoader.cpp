#ifndef ASSET_LOADER_CPP
#define ASSET_LOADER_CPP

#include "AssetLoader.h"

namespace RacingEngine {

AAssetManager* AssetLoader::assetManager = nullptr;

bool AssetLoader::initialize(AAssetManager* manager) {
    assetManager = manager;
    return assetManager != nullptr;
}

ModelData AssetLoader::loadModel(const std::string& path) {
    ModelData result;
    result.size = 0;

    if (!assetManager) return result;

    AAsset* asset = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset) return result;

    result.size = AAsset_getLength(asset);
    result.data.resize(result.size);
    AAsset_read(asset, result.data.data(), result.size);
    AAsset_close(asset);

    return result;
}

std::vector<unsigned char> AssetLoader::loadTexture(const std::string& path) {
    std::vector<unsigned char> data;

    if (!assetManager) return data;

    AAsset* asset = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset) return data;

    size_t size = AAsset_getLength(asset);
    data.resize(size);
    AAsset_read(asset, data.data(), size);
    AAsset_close(asset);

    return data;
}

std::string AssetLoader::loadConfig() {
    std::string config;

    if (!assetManager) return config;

    AAsset* asset = AAssetManager_open(assetManager, "asset_config.json", AASSET_MODE_BUFFER);
    if (!asset) return config;

    size_t size = AAsset_getLength(asset);
    config.resize(size);
    AAsset_read(asset, config.data(), size);
    AAsset_close(asset);

    return config;
}

} // namespace RacingEngine

#endif
