//
// Created by felix on 30/04/2021.
//

#ifndef G2_ASSET_REGISTRY_H
#define G2_ASSET_REGISTRY_H

#include <cstdint>
#include <span>
#include <vector>
#include <filesystem>
#include <unordered_map>

namespace g2 {

    struct AssetReferencePatch {
        const char* name;
        uint64_t* index;
    };

    struct AssetAddResult {
        uint64_t index;
        std::span<AssetReferencePatch> patches;
    };

    struct IAssetManager {
        virtual AssetAddResult add_asset(void *) = 0;
        virtual const char* ext() = 0;
    };

    class AssetRegistry {
    public:
        void includePath(const char* path);
        void registerManager(IAssetManager* assetManager);
        uint64_t getAssetIndex(const char* name);
    private:
        IAssetManager* findAssetManager(const std::filesystem::path& path);

        std::vector<std::filesystem::path> searchPaths;
        std::vector<IAssetManager*> assetManagers;
        std::unordered_map<std::string, uint64_t> assetMap;
    };

}
#endif //G2_ASSET_REGISTRY_H
