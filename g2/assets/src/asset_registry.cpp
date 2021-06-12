//
// Created by felix on 30/04/2021.
//

#include <asset_registry.h>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <g2/archive/g2archive.h>
#include <g2/file.h>

namespace fs = std::filesystem;

void g2::AssetRegistry::registerManager(IAssetManager *assetManager) {
    assetManagers.push_back(assetManager);
}

uint32_t g2::AssetRegistry::getAssetIndex(const char *name) {
    auto f = assetMap.find(name);
    assert(f != assetMap.end());
    return f->second;
}

void g2::AssetRegistry::includePath(const char* pathStr) {

    std::vector<AssetReferencePatch> patches;

    fs::path path(pathStr);

    auto& sources = sourcesMap[path];

    for(auto& p : fs::recursive_directory_iterator(path)) {
        if (strcmp(p.path().extension().c_str(), ".g2ar") == 0) {
            //std::cout << p << std::endl;

           auto bytes = map(p.path().c_str());
           sources.push_back({p.path(), bytes});

            auto archive = archive::GetArchive(bytes.data());

            for(auto entry : *archive->entries()) {
                //std::cout << "\t" << entry->path()->c_str() << std::endl;
                if (auto m = findAssetManager(fs::path(entry->path()->c_str()))) {
                    auto contents = std::span((char*)entry->contents()->data(), entry->contents()->size());
                    AssetAddResult result = m->add_asset(contents);

                    for(auto patch : result.patches) {
                        //std::cout << "\t\t@" << patch.name << std::endl;
                        patches.push_back(AssetReferencePatch {
                            .name = p.path().parent_path() / p.path().stem() / patch.name,
                            .index = patch.index,
                        });
                    }

                    auto name = p.path().parent_path() / p.path().stem() / entry->path()->c_str();
                    assetMap.emplace(name.c_str(), result.index);
                }
            }


        } else if(auto m = findAssetManager(p.path())) {
            //std::cout << p << std::endl;

            auto bytes = map(p.path().c_str());
            sources.push_back({p.path(), bytes});

            AssetAddResult result = m->add_asset(bytes);
            for(auto patch : result.patches) {
                //std::cout << "\t\t@" << patch.name << std::endl;
                patches.push_back(AssetReferencePatch {
                        .name = p.path().parent_path() / p.path().stem() / patch.name,
                        .index = patch.index,
                });
            }
            assetMap.emplace(p.path().c_str(), result.index);
        }
    }

    for(auto& patch : patches) {
        *patch.index = getAssetIndex(patch.name.lexically_normal().c_str());
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
