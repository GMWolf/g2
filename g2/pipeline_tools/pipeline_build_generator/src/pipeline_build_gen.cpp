//
// Created by felix on 01/05/2021.
//

#include <filesystem>
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <span>
#include "ninja.h"
#include <fstream>


namespace fs = std::filesystem;


void processGltf(ninjactx& ctx, const fs::path& path) {

    cgltf_options options {};
    cgltf_data* data {};
    cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);

    //output dir for this gltf relative to build dir
    auto outDir = fs::relative(path, ctx.sourceDir);

    auto inPath = fs::relative(path, ctx.buildDir);

    if (result != cgltf_result_success) {
        std::cerr << "Error loading file " << path << "\n";
        return;
    }

    // add image build statements
    {
        for(auto image : std::span(data->images, data->images + data->images_count)) {
            auto output = (outDir / image.uri).replace_extension(".ktx2");
            auto input = inPath.parent_path() / image.uri;
            const char* outputs[] = {output.c_str()};
            const char* inputs[] = {input.c_str()};
            write_build(ctx, "toktx", outputs, inputs);
        }
    }

    //add mesh build statements
    {
        for(auto mesh : std::span(data->meshes, data->meshes + data->meshes_count)) {
            auto output =  (outDir / mesh.name).concat(".g2mesh");
            const char* outputs[] = {output.c_str()};
            const char* inputs[] = {inPath.c_str()};

            std::pair<const char*, const char*> flags[] {
                    {"mesh", mesh.name},
            };

            write_build(ctx, "g2mesh", outputs, inputs, flags);
        }
    }

    //add mat build statements
    {
        auto output =  outDir / "materials.g2mat";
        const char* outputs[] = {output.c_str()};
        const char* inputs[] = {inPath.c_str()};
        write_build(ctx, "g2mat", outputs, inputs);
    }


    cgltf_free(data);
}

void processPipeline(ninjactx& ctx, const fs::path& path) {

}

int main(int argc, char* argv[]) {


    fs::path sourcePath = argv[1];
    fs::path buildDir = argv[2];

    fs::create_directories(buildDir);

    std::ofstream out(buildDir/"build.ninja");

    ninjactx ctx {
        .out = out,
        .buildDir = buildDir,
        .sourceDir = sourcePath,
    };

    write_rule(ctx, "toktx", TOKTX_PATH " --uastc 0 --uastc_rdo_l --zcmp 1 --genmipmap $out $in");
    write_rule(ctx, "g2mesh", G2MESH_PATH " $in $mesh $out");
    write_rule(ctx, "g2mat", G2MAT_PATH " $in $out");
    write_rule(ctx, "g2shader", G2SHADER_PATH " $in $out");


    for(auto& p : fs::recursive_directory_iterator(sourcePath)) {

        auto path = p.path();
        std::cout << path << std::endl;

        if (strcmp(path.extension().c_str(), ".gltf") == 0) {
            processGltf(ctx, path);
        } else if (strcmp(path.extension().c_str(), ".pipeline") == 0) {

        }

    }





}