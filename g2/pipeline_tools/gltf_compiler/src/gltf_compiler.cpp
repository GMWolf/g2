//
// Created by felix on 25/04/2021.
//

#include <iostream>
#include <span>
#include <cstring>
#include <algorithm>

#include <filesystem>

#include <cgltf.h>

#include <cstdint>
#include <cassert>

#include <flatbuffers/flatbuffers.h>
#include <g2/gfx/mesh_generated.h>
#include <fstream>
#include <regex>

#include <g2/archive/g2archive.h>

#include "mesh_compiler.h"
#include "material_compiler.h"

namespace fs = std::filesystem;
namespace fb = flatbuffers;


int main(int argc, char* argv[]) {

    const fs::path input = argv[1];
    const fs::path output = argv[2];

    const fs::path directory = input.parent_path();

    cgltf_options options = {};
    cgltf_data* data{};
    cgltf_result result = cgltf_parse_file(&options, input.c_str(), &data);

    if(result != cgltf_result_success) {
        std::cerr << "Error loading file: " << input << "\n";
        return 1;
    }

    result = cgltf_load_buffers(&options, data, input.c_str());
    if(result != cgltf_result_success) {
        std::cerr << "Error loading buffers for " << input << "error code: " << result << "\n";
        return 1;
    }

    g2::archive::ArchiveWriter archiveWriter;

    for(auto mesh : std::span(data->meshes, data->meshes_count)) {
        auto meshData = compileMesh(&mesh);
        auto meshPath = fs::path(mesh.name).concat(".g2mesh");
        archiveWriter.addEntry(meshPath.c_str(), meshData);
    }

    for(auto mat : std::span(data->materials, data->materials_count)) {
        auto matData = compileMaterial(&mat);

        auto matPath = fs::path(mat.name).concat(".g2mat");

        archiveWriter.addEntry(matPath.c_str(), matData);
    }


    auto archiveData = archiveWriter.finish();
    fs::create_directories(output.parent_path());
    std::ofstream archiveOutput(output.c_str(), std::ios::out | std::ios::binary);
    archiveOutput.write((char*)archiveData.data(), archiveData.size());

    {
        std::ofstream ofs(output.string() + ".d");
        std::regex whitespaceRegex("\\s");

        ofs << output.c_str() << " :";

        for(cgltf_buffer& buffer : std::span(data->buffers, data->buffers_count)) {
            ofs << " " << std::regex_replace((directory / buffer.uri).c_str(), whitespaceRegex, "\\$&");
        }
    }



    cgltf_free(data);

    return 0;
}