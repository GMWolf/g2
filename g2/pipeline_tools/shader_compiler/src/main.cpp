//
// Created by felix on 19/04/2021.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <flatbuffers/flatbuffers.h>
#include <g2/gfx/pipeline_generated.h>
#include <rapidjson/document.h>
#include <g2/file.h>
#include <shaderc/shaderc.hpp>
#include <g2/fbutil.h>
#include <flatbuffers/util.h>
#include <regex>
#include "module.h"
#include "reflection.h"

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
    std::vector<flatbuffers::Offset<g2::gfx::ShaderModule>> fbmodules;

    std::vector<std::filesystem::path> deps;

    auto &shader = doc["shader"];

    if(shader.HasMember("vertex")) {
        auto vertexFile = inputDir / shader["vertex"].GetString();
        deps.push_back(vertexFile);
        std::string vertexSource;
        g2::readFile(vertexFile.c_str(), vertexSource);

        auto result = compileModule(vertexSource.c_str(), vertexFile.c_str(), g2::gfx::ShaderStage::vertex, deps);
        if (result.empty()) {
            return 1;
        }

        auto fbVertexText = fbb.CreateVector(result.data(), result.size());
        auto fbVertexModule = g2::gfx::CreateShaderModule(fbb, fbVertexText, g2::gfx::ShaderStage::vertex);
        fbmodules.push_back(fbVertexModule);
    }

    if (shader.HasMember("fragment")) {
        auto fragmentFile = inputDir / shader["fragment"].GetString();
        deps.push_back(fragmentFile);
        std::string fragmentSource;
        g2::readFile(fragmentFile.c_str(), fragmentSource);

        auto result = compileModule(fragmentSource.c_str(), fragmentFile.c_str(), g2::gfx::ShaderStage::fragment, deps);
        if (result.empty()){
            return 1;
        }

        auto fbFragmentText = fbb.CreateVector(result.data(), result.size());
        auto fbFragmentModule = g2::gfx::CreateShaderModule(fbb, fbFragmentText, g2::gfx::ShaderStage::fragment);
        fbmodules.push_back(fbFragmentModule);
    }


    auto &blending = doc["blending"];
    assert(blending.IsArray());
    auto &attachmentsValue = doc["attachments"];
    assert(attachmentsValue.IsArray());

    std::vector<g2::gfx::BlendAttachment> blend_attachments(blending.Size());
    for (size_t i = 0; i < blending.Size(); i++) {

        auto &b = blending[i];

        g2::gfx::BlendAttachment blend_attachment(
                b["blend_enable"].GetBool(),
                static_cast<g2::gfx::BlendFactor>(g2::enumLookup(b["src_blend_factor"].GetString(),
                                                                 g2::gfx::BlendFactorTypeTable()).value()),
                static_cast<g2::gfx::BlendFactor>(g2::enumLookup(b["dst_blend_factor"].GetString(),
                                                                 g2::gfx::BlendFactorTypeTable()).value()),
                static_cast<g2::gfx::BlendOp>(g2::enumLookup(b["color_blend_op"].GetString(),
                                                             g2::gfx::BlendOpTypeTable()).value()),
                static_cast<g2::gfx::BlendFactor>(g2::enumLookup(b["src_alpha_factor"].GetString(),
                                                                 g2::gfx::BlendFactorTypeTable()).value()),
                static_cast<g2::gfx::BlendFactor>(g2::enumLookup(b["dst_alpha_factor"].GetString(),
                                                                 g2::gfx::BlendFactorTypeTable()).value()),
                static_cast<g2::gfx::BlendOp>(g2::enumLookup(b["alpha_blend_op"].GetString(),
                                                             g2::gfx::BlendOpTypeTable()).value())
        );

        blend_attachments[i] = blend_attachment;
    }


    std::vector<g2::gfx::Attachment> attachments(attachmentsValue.Size());
    for (size_t i = 0; i < attachmentsValue.Size(); i++) {
        auto formatStr = attachmentsValue[i]["format"].GetString();

        g2::gfx::Format format = static_cast<g2::gfx::Format>(g2::enumLookup(formatStr, g2::gfx::FormatTypeTable()).value());

        attachments[i] = g2::gfx::Attachment(format);
    }

    g2::gfx::Attachment depthAttachment;
    g2::gfx::Attachment* pDepthAttachment = nullptr;

    if(doc.HasMember("depthAttachment")) {
        depthAttachment = static_cast<g2::gfx::Format>(g2::enumLookup(doc["depthAttachment"]["format"].GetString(), g2::gfx::FormatTypeTable()).value());
        pDepthAttachment = &depthAttachment;
    }

    g2::gfx::CullMode cullMode = static_cast<g2::gfx::CullMode>(g2::enumLookup(doc["cullMode"].GetString(), g2::gfx::CullModeTypeTable()).value());
    g2::gfx::CompareOp depthCompare = static_cast<g2::gfx::CompareOp>(g2::enumLookup(doc["depthCompare"].GetString(), g2::gfx::CompareOpTypeTable()).value());

    auto fbpipeline = g2::gfx::CreatePipelineDefDirect(fbb, &fbmodules, &blend_attachments, &attachments, pDepthAttachment, cullMode, depthCompare);
    fbb.Finish(fbpipeline);

    {
        auto buf = fbb.GetBufferPointer();
        auto buf_size = fbb.GetSize();
        std::ofstream ofs(outputPath.c_str(), std::ios::out | std::ios::binary);
        ofs.write((char *) buf, buf_size);
    }

    //Write dep file
    {
        std::ofstream ofs(outputPath.string() + ".d");
        std::regex whitespaceRegex("\\s");
        ofs << outputPath.c_str() << " :";

        for(auto d : deps) {
            auto escapedPath = std::regex_replace(d.string(), whitespaceRegex, "\\$&");
            ofs << " " << escapedPath;
        }
    }

    return 0;

}