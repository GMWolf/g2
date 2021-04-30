//
// Created by felix on 30/04/2021.
//

#include <asset_registry.h>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void g2::AssetRegistry::registerManager(IAssetManager *assetManager) {
    assetManagers.push_back(assetManager);
}

uint64_t g2::AssetRegistry::getAssetIndex(const char *name) {
    auto f = assetMap.find(name);
    assert(f != assetMap.end());
    return f->second;
}

void g2::AssetRegistry::includePath(const char* pathStr) {

    std::vector<AssetReferencePatch> patches;

    auto& path = searchPaths.emplace_back(pathStr);
    for(auto& p : fs::recursive_directory_iterator(path)) {
        std::cout  << p.path().c_str() << std::endl;
        if(auto m = findAssetManager(p.path())) {
            std::ifstream stream(p.path().c_str(), std::ios::binary);
            std::vector<char> bytes((std::istreambuf_iterator<char>(stream)),
                                        (std::istreambuf_iterator<char>()));
            AssetAddResult result = m->add_asset(bytes.data());
            patches.insert(patches.end(), result.patches.begin(), result.patches.end());
            assetMap.emplace(p.path().c_str(), result.index);
        }
    }

}

g2::IAssetManager *g2::AssetRegistry::findAssetManager(const fs::path& path) {

    for(auto m : assetManagers) {
        if (strcmp(path.extension().c_str(), m->ext()) == 0) {
            return m;
        }
    }

    return nullptr;
}
