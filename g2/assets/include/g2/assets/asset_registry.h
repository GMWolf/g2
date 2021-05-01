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
        uint32_t* index;
    };

    struct AssetAddResult {
        uint32_t index;
        std::vector<AssetReferencePatch> patches;
    };

    struct IAssetManager {
        virtual AssetAddResult add_asset(std::span<char> data) = 0;
        virtual const char* ext() = 0;
    };

    class AssetRegistry {
    public:
        void includePath(const char* path);
        void registerManager(IAssetManager* assetManager);
        uint32_t getAssetIndex(const char* name);
    private:
        IAssetManager* findAssetManager(const std::filesystem::path& path);

        std::vector<std::filesystem::path> searchPaths;
        std::vector<IAssetManager*> assetManagers;
        std::unordered_map<std::string, uint32_t> assetMap;
    };

}
#endif //G2_ASSET_REGISTRY_H
