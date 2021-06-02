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

namespace fs = std::filesystem;
namespace json = rapidjson;

struct ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {

    struct ResultContainer {
        shaderc_include_result result;
        std::string sourceName;
        std::string contents;
    };

    shaderc_include_result *
    GetInclude(const char *requested_source, shaderc_include_type type, const char *requesting_source,
               size_t include_depth) override {
        auto path = fs::path(requesting_source).parent_path() / requested_source;
        std::ifstream stream(path);

        auto result = new ResultContainer;
        result->sourceName = path.filename().string();

        if (stream) {
            result->contents = std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            result->result.content = result->contents.c_str();
            result->result.content_length = result->contents.length();
            result->result.source_name = result->sourceName.c_str();
            result->result.source_name_length = result->sourceName.length();
        } else {
            result->result.content = nullptr;
            result->result.content_length = 0;
            result->result.source_name = nullptr;
            result->result.source_name_length = 0;
        }

        result->result.user_data = result;


        return &result->result;
    }

    void ReleaseInclude(shaderc_include_result *data) override {
        delete (ResultContainer *) data->user_data;
    }
};


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

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetIncluder(std::make_unique<ShaderIncluder>());
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

    flatbuffers::FlatBufferBuilder fbb(2048);
    std::vector<flatbuffers::Offset<g2::gfx::ShaderModule>> fbmodules;

    std::vector<fs::path> deps;

    auto &shader = doc["shader"];

    if(shader.HasMember("vertex")) {
        auto vertexFile = inputDir / shader["vertex"].GetString();
        deps.push_back(vertexFile);
        std::string vertexSource;
        g2::readFile(vertexFile.c_str(), vertexSource);
        auto vertexPreprocessResult = compiler.PreprocessGlsl(vertexSource, shaderc_vertex_shader, vertexFile.c_str(),
                                                              options);
        if (vertexPreprocessResult.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cerr << vertexPreprocessResult.GetErrorMessage() << "\n";
            return -1;
        }
        auto vertexResult = compiler.CompileGlslToSpv({vertexPreprocessResult.cbegin(), vertexPreprocessResult.cend()},
                                                      shaderc_vertex_shader, vertexFile.c_str(), options);
        if (vertexResult.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cerr << vertexResult.GetErrorMessage() << "\n";
            return -1;
        }
        size_t vertexWordSize = std::distance(vertexResult.cbegin(), vertexResult.cend());
        auto fbVertexText = fbb.CreateVector(vertexResult.cbegin(), vertexWordSize);
        auto fbVertexModule = g2::gfx::CreateShaderModule(fbb, fbVertexText, g2::gfx::ShaderStage::vertex);
        fbmodules.push_back(fbVertexModule);
    }

    if (shader.HasMember("fragment")) {
        auto fragmentFile = inputDir / shader["fragment"].GetString();
        deps.push_back(fragmentFile);
        std::string fragmentSource;
        g2::readFile(fragmentFile.c_str(), fragmentSource);
        auto fragmentPreprocessResult = compiler.PreprocessGlsl(fragmentSource, shaderc_fragment_shader,
                                                                fragmentFile.c_str(), options);
        if (fragmentPreprocessResult.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cerr << fragmentPreprocessResult.GetErrorMessage() << "\n";
            return -1;
        }
        auto fragmentResult = compiler.CompileGlslToSpv(
                {fragmentPreprocessResult.cbegin(), fragmentPreprocessResult.cend()}, shaderc_fragment_shader,
                fragmentFile.c_str(), options);
        if (fragmentResult.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cerr << fragmentResult.GetErrorMessage() << "\n";
            return -1;
        }

        size_t fragmentWordSize = std::distance(fragmentResult.cbegin(), fragmentResult.cend());
        auto fbFragmentText = fbb.CreateVector(fragmentResult.cbegin(), fragmentWordSize);
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

    g2::gfx::Attachment depthAttachment(static_cast<g2::gfx::Format>(g2::enumLookup(doc["depthAttachment"]["format"].GetString(), g2::gfx::FormatTypeTable()).value()));

    auto fbpipeline = g2::gfx::CreatePipelineDefDirect(fbb, &fbmodules, &blend_attachments, &attachments, &depthAttachment);
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