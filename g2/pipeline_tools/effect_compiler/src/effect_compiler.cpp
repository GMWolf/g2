//
// Created by felix on 02/06/2021.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <g2/gfx/effect_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <rapidjson/document.h>
#include <g2/file.h>
#include <g2/fbutil.h>
#include <flatbuffers/util.h>

namespace fs = std::filesystem;
namespace json = rapidjson;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Wrong number of arguments received. 2 expected.\n";
        return 1;
    }

    fs::path inputPath(argv[1]);
    fs::path outputPath(argv[2]);

    fs::path inputDir = inputPath.parent_path();

    json::Document doc;
    std::string docText;
    g2::readFile(inputPath.c_str(), docText);
    doc.ParseInsitu(docText.data());

    flatbuffers::FlatBufferBuilder fbb(2048);

    std::vector<flatbuffers::Offset<g2::gfx::EffectPass>> effectPasses;
    for(auto it = doc.MemberBegin(); it != doc.MemberEnd(); it++) {
        fs::path pipelineName = (fs::path("..") / it->value.GetString()).concat(".g2ppln");
        auto pass = g2::gfx::CreateEffectPassDirect(fbb, it->name.GetString(), pipelineName.c_str());
        effectPasses.push_back(pass);
    }

    auto effect = g2::gfx::CreateEffectDefDirect(fbb, &effectPasses);

    fbb.Finish(effect);

    {
        auto buf = fbb.GetBufferPointer();
        auto buf_size = fbb.GetSize();
        std::ofstream ofs(outputPath.c_str(), std::ios::out | std::ios::binary);
        ofs.write((char *) buf, buf_size);
    }

}